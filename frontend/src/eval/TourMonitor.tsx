import * as React from "react";
import {
  BalancerObservable,
  ImplementationObservable,
  ViewCenterObservable,
  WorkerObservable,
  IterationObservable
} from "../misc/Observable";
import { Point3D } from "../misc/Point";
import { StatsCollector } from "./StatsCollector";
import { Tour, Output, PoI } from "./ConfigTypes";

import "./TourMonitor.css";
import { Balancers, Implementations } from "../Constants";

interface Config {
  run: number;
  balancer: string;
  implementation: string;
  maxIteration: number;
  nodeCount: number;
  poi: PoI;
}

interface TourMonitorProps {
  stats: StatsCollector;
  viewCenter: ViewCenterObservable;
  balancer: BalancerObservable;
  impl: ImplementationObservable;
  workerCount: WorkerObservable;
  iter: IterationObservable;
}
interface TourMonitorState {
  running: boolean;
  progress: number;
  configLength: number;
  output: string;
  currentConfig: Config;
}

export default class TourMonitor extends React.Component<TourMonitorProps, TourMonitorState> {
  private configs: Config[];
  private map: HTMLElement;

  constructor(props: TourMonitorProps) {
    super(props);
    this.onFileChange = this.onFileChange.bind(this);
    this.state = {
      run: 1,
      running: false,
      progress: 0,
      configLength: 0,
      output: "",
      currentConfig: {
        balancer: "",
        implementation: "",
        maxIteration: 0,
        nodeCount: 0,
        poi: {
          real: 0,
          imag: 0,
          zoom: 0
        }
      }
    };
  }

  public render() {
    const progress = `${this.state.progress.toFixed(2)}%`;
    return (
      <div className="tour">
        <h2>Tour {this.state.running ? "running..." : "done"}</h2>
        <div className="progress">
          <div
            className="progress-bar"
            role="progressbar"
            aria-valuenow={this.state.progress.toFixed(2)}
            aria-valuemin={0}
            aria-valuemax={100}
            style={{ width: progress }}
          >
            {progress}
          </div>
        </div>
        <div>
          <table>
            <tbody>
              <tr>
                <td>
                  <p>Configuration file:</p>
                </td>
                <td>
                  <input
                    type="file"
                    onChange={e => this.onFileChange(e.target.files as FileList)}
                  />
                </td>
              </tr>
            </tbody>
          </table>
        </div>
        <p>currently running configuration:</p>
        <RenderConfig {...this.state.currentConfig} />
        <p>JSON output (triple click to select all):</p>
        <pre className="jsonOutput">{this.state.output}</pre>
      </div>
    );
  }

  private onFileChange(files: FileList) {
    let fr = new FileReader();
    fr.onload = () => this.start(fr.result as string);
    fr.readAsText(files[0]);
  }

  private start(configJSON: string) {
    // generate config combinations
    const config = JSON.parse(configJSON) as Tour;
    this.configs = [];
    for (let run = 1; run <= config.runs; run++) {
      for (const balancer of config.balancers) {
        if (!Balancers.some(b => b.key === balancer)) {
          console.error("Invalid balancer: ", balancer);
          return;
        }
        for (const implementation of config.implementations) {
          if (!Implementations.some(i => i.key === implementation)) {
            console.error("Invalid implementation: ", balancer);
            return;
          }
          if (config.maxIteration.length !== 3 && config.maxIteration.length !== 1) {
            console.error("Invalid maxIteration specification: ", config.maxIteration);
            return;
          }
          let min_I = config.maxIteration[0];
          let max_I = config.maxIteration[0];
          let step_I = config.maxIteration[0];
          if (config.maxIteration.length === 3) {
            max_I = config.maxIteration[1];
            step_I = config.maxIteration[2];
          }
          for (let maxIteration = min_I; maxIteration <= max_I; maxIteration += step_I) {
            if (config.nodeCount.length !== 3 && config.nodeCount.length !== 1) {
              console.error("Invalid nodeCount specification: ", config.nodeCount);
              return;
            }
            let min_N = config.nodeCount[0];
            let max_N = config.nodeCount[0];
            let step_N = config.nodeCount[0];
            if (config.maxIteration.length === 3) {
              max_N = config.nodeCount[1];
              step_N = config.nodeCount[2];
            }
            for (let nodeCount = min_N; nodeCount <= max_N; nodeCount += step_N) {
              for (const poi of config.pois) {
                this.configs.push({
                  run,
                  balancer,
                  implementation,
                  maxIteration,
                  nodeCount,
                  poi
                });
              }
            }
          }
        }
      }
    }

    console.log("generated configs: ", this.configs);

    const out: Output = { config, datapoints: [] };

    // set size of leaflet container
    this.map = document.getElementById("viewer") as HTMLElement;
    this.map.style.width = `${config.screen.width}px`;
    this.map.style.height = `${config.screen.height}px`;

    // reset stats
    this.props.stats.done();
    this.setState(state =>
      Object.assign(state, {
        running: true,
        currentConfig: this.configs[0],
        configLength: this.configs.length
      })
    );
    this.runConfig(out, this.configs);
  }

  private runConfig(output: Output, configs: Config[]) {
    // tour is done
    if (configs.length === 0) {
      this.done(output);
      return;
    }
    const c = configs[0] as Config;
    console.log("Tour: Changing config to: ", c);

    // // only notify the observable that has changed once
    // const oldBl = this.props.balancer.get();
    // const oldImp = this.props.impl.get();
    // const oldIter = this.props.iter.get();
    // const oldCenter = this.props.viewCenter.get();

    this.props.balancer.setNoNotify(c.balancer);
    this.props.impl.setNoNotify(c.implementation);
    this.props.iter.setNoNotify(c.maxIteration);
    this.props.workerCount.setNoNotify(c.nodeCount);
    const pt = new Point3D(c.poi.real, c.poi.imag, c.poi.zoom);
    if (!this.props.viewCenter.set(pt)) {
      this.props.balancer.notify();
    }

    // notify all view observers

    this.props.stats.onDone(stats => {
      output.datapoints.push({
        setting: {
          balancer: c.balancer,
          implementation: c.implementation,
          maxIteration: c.maxIteration,
          nodeCount: c.nodeCount,
          run: c.run
        },
        poi: c.poi,
        data: {
          balancer: {
            time: stats.balancer.time,
            regionCount: stats.balancer.regionCount
          },
          workers: Array.from(stats.worker.values())
        }
      });

      this.setState(state =>
        Object.assign(state, {
          progress: ((state.configLength - configs.length + 1) * 100) / state.configLength,
          output: JSON.stringify(output),
          currentConfig: c
        })
      );
      console.log("current stats: ", JSON.stringify(output));
      this.runConfig(output, configs.slice(1));
    });
  }

  private done(output: Output) {
    // reset leaflet container to original size
    this.map.style.width = "";
    this.map.style.height = "";

    console.log("Stats collection is done: ", JSON.stringify(output));
    this.setState(state =>
      Object.assign(state, { output: JSON.stringify(output), running: false })
    );
  }
}

const RenderConfig = (props: Config) => {
  return (
    <table>
      <tbody>
        {Tr("Balancer:", props.balancer)}
        {Tr("Implementation:", props.implementation)}
        {Tr("Nodes:", props.nodeCount)}
        {Tr("max Iteration:", props.maxIteration)}
        <tr>
          <td>PoI</td>
          <td>
            <pre className="config">
              real: {props.poi.real.toFixed(2)}, imag: {props.poi.imag.toFixed(2)}, zoom:{" "}
              {props.poi.imag.toFixed(2)}
            </pre>
          </td>
        </tr>
      </tbody>
    </table>
  );
};

const Tr = (name: string, val: any) => (
  <tr>
    <td>{name}</td>
    <td>
      <pre className="config">{JSON.stringify(val)}</pre>
    </td>
  </tr>
);
