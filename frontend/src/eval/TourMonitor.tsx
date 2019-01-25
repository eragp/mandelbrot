import * as React from "react";
import {
  BalancerObservable,
  ImplementationObservable,
  ViewCenterObservable,
  WorkerObservable,
  IterationObservable,
  PredAccObservable,
  RunObservable
} from "../misc/Observable";
import { Point3D } from "../misc/Point";
import { StatsCollector } from "./StatsCollector";
import { Tour, Output, PoI } from "./ConfigTypes";

import "./TourMonitor.css";
import { Balancers, Implementations } from "../Constants";

interface Config {
  run: number;
  predictionAccuracy: number;
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
  predAcc: PredAccObservable;
  run: RunObservable;
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
      running: false,
      progress: 0,
      configLength: 0,
      output: "",
      currentConfig: {
        run: 1,
        predictionAccuracy: 0,
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
            aria-valuenow={Math.round(this.state.progress * 100) / 100}
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
    const fr = new FileReader();
    fr.onload = () => this.start(fr.result as string);
    if (files.length > 0) {
      fr.readAsText(files[0]);
    }
  }

  private start(configJSON: string) {
    // generate config combinations
    const config = JSON.parse(configJSON) as Tour;
    this.configs = [];
    for (let run = 1; run <= config.runs; run++) {
      if (!config.balancers) {
        console.error("Invalid balancers: undefined");
        return;
      }
      for (const balancer of config.balancers) {
        if (!Balancers.some(b => b.key === balancer)) {
          console.error("Invalid balancer: ", balancer);
          return;
        }
        if (!config.implementations) {
          console.error("Invalid implementations: undefined");
          return;
        }
        for (const implementation of config.implementations) {
          if (!Implementations.some(i => i.key === implementation)) {
            console.error("Invalid implementation: ", balancer);
            return;
          }
          if (!config.maxIteration) {
            console.error("Invalid maxIterations: undefined");
            return;
          }
          if (config.maxIteration.length !== 3 && config.maxIteration.length !== 1) {
            console.error("Invalid maxIteration specification: ", config.maxIteration);
            return;
          }
          let minI = config.maxIteration[0];
          let maxI = config.maxIteration[0];
          let stepI = config.maxIteration[0];
          if (config.maxIteration.length === 3) {
            maxI = config.maxIteration[1];
            stepI = config.maxIteration[2];
          }
          for (let maxIteration = minI; maxIteration <= maxI; maxIteration += stepI) {
            if (!config.nodeCount) {
              console.error("Invalid nodeCount: undefined");
              return;
            }
            if (config.nodeCount.length !== 3 && config.nodeCount.length !== 1) {
              console.error("Invalid nodeCount specification: ", config.nodeCount);
              return;
            }
            let minN = config.nodeCount[0];
            let maxN = config.nodeCount[0];
            let stepN = config.nodeCount[0];
            if (config.nodeCount.length === 3) {
              maxN = config.nodeCount[1];
              stepN = config.nodeCount[2];
            }
            if (!config.predictionAccuracy) {
              console.error("Invalid predictionAccuracy: undefined");
              return;
            }
            if (config.predictionAccuracy.length !== 3 && config.predictionAccuracy.length !== 1) {
              console.error(
                "Invalid predictionAccuracy specification: ",
                config.predictionAccuracy
              );
              return;
            }
            let minP = config.predictionAccuracy[0];
            let maxP = config.predictionAccuracy[0];
            let stepP = 1;
            if (config.predictionAccuracy.length === 3) {
              maxP = config.predictionAccuracy[1];
              stepP = config.predictionAccuracy[2];
            }
            for (
              let predictionAccuracy = minP;
              predictionAccuracy <= maxP;
              predictionAccuracy += stepP
            ) {
              if (predictionAccuracy === 0) {
                continue;
              }
              for (let nodeCount = minN; nodeCount <= maxN; nodeCount += stepN) {
                for (const poi of config.pois) {
                  this.configs.push({
                    run,
                    predictionAccuracy,
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

    this.props.balancer.setNoNotify(c.balancer);
    this.props.impl.setNoNotify(c.implementation);
    this.props.iter.setNoNotify(c.maxIteration);
    this.props.workerCount.setNoNotify(c.nodeCount);
    this.props.predAcc.setNoNotify(c.predictionAccuracy);
    this.props.run.setNoNotify(c.run);
    const pt = new Point3D(c.poi.real, c.poi.imag, c.poi.zoom);
    this.props.viewCenter.setNoNotify(pt);
    this.props.balancer.notify();

    // notify all view observers

    this.props.stats.onDone(stats => {
      output.datapoints.push({
        setting: {
          balancer: c.balancer,
          implementation: c.implementation,
          maxIteration: c.maxIteration,
          predictionAccuracy: c.predictionAccuracy,
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
