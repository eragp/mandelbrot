import * as React from "react";
import {
  BalancerObservable,
  ImplementationObservable,
  ViewCenterObservable
} from "../misc/Observable";
import { Point3D } from "../misc/Point";
import { StatsCollector } from "./StatsCollector";

import "./TourMonitor.css";
import { doesNotReject } from "assert";

interface Tour {
  screen: ScreenOpts;
  balancers: string[];
  implementations: string[];
  nodeCount: number;
  description: string;
  pois: PoI[];
}
interface ScreenOpts {
  width: number;
  height: number;
}
interface PoI {
  real: number;
  imag: number;
  zoom: number;
}

interface Output {
  config: Tour;
  datapoints: Run[];
}
interface Run {
  balancer: string;
  implementation: string;
  data: Datapoint;
}
interface Datapoint {
  poi: PoI;
  balancer: BalancerTime;
  workers: WorkerTime[];
}
interface BalancerTime {
  time: number;
  regionCount: number;
}
interface WorkerTime {
  rank: number;
  computationTime: number;
  mpiTime: number;
  drawTime: number;
}

interface Config {
  balancer: string;
  impl: string;
  poi: PoI;
}

interface TourMonitorProps {
  stats: StatsCollector;
  viewCenter: ViewCenterObservable;
  balancer: BalancerObservable;
  impl: ImplementationObservable;
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
    // read configs
    const config = require("./config.json") as Tour;

    // generate config combinations
    this.configs = [];
    for (const balancer of config.balancers) {
      for (const impl of config.implementations) {
        for (const poi of config.pois) {
          this.configs.push({ balancer, impl, poi });
        }
      }
    }

    const out: Output = { config, datapoints: [] };
    this.state = {
      running: true,
      progress: 0,
      configLength: this.configs.length,
      output: "",
      currentConfig: this.configs[0]
    };

    // set size of leaflet container
    this.map = document.getElementById("viewer") as HTMLElement;
    this.map.style.width = `${config.screen.width}px`;
    this.map.style.height = `${config.screen.height}px`;

    // reset stats
    this.props.stats.done();
    this.runConfig(out, this.configs);
  }

  public render() {
    const progress = `${this.state.progress}%`;
    return (
      <div className="tour">
        <h2>Tour {this.state.running ? "running..." : "done"}</h2>
        <div className="progress">
          <div
            className="progress-bar"
            role="progressbar"
            aria-valuenow={this.state.progress}
            aria-valuemin={0}
            aria-valuemax={100}
            style={{ width: progress }}
          >
            {progress}
          </div>
        </div>
        <p>currently running configuration:</p>
        <RenderConfig {...this.state.currentConfig} />
        <p>JSON output (triple click to select all):</p>
        <pre className="jsonOutput">{this.state.output}</pre>
      </div>
    );
  }

  private done(output: Output) {
    // reset leaflet container to original size
    this.map.style.width = "";
    this.map.style.height = "";

    console.log("Stats collection is done");
    console.log(JSON.stringify(output));
    this.setState(state =>
      Object.assign(state, { output: JSON.stringify(output), running: false })
    );
  }

  private runConfig(output: Output, configs: Config[]) {
    // tour is done
    if (configs.length === 0) {
      this.done(output);
      return;
    }
    const c = configs.pop() as Config;
    console.log("Tour: Changing config to: ", c);

    const oldBl = this.props.balancer.get();
    const oldImp = this.props.impl.get();

    this.props.balancer.setNoNotify(c.balancer);
    this.props.impl.setNoNotify(c.impl);
    this.props.viewCenter.setNoNotify(new Point3D(c.poi.real, c.poi.imag, c.poi.zoom));
    if (oldBl !== c.balancer) {
      this.props.balancer.notify();
    } else if (oldImp !== c.impl) {
      this.props.impl.notify();
    } else {
      this.props.viewCenter.notify();
    }
    this.props.stats.onDone(stats => {
      output.datapoints.push({
        balancer: c.balancer,
        implementation: c.impl,
        data: {
          poi: c.poi,
          balancer: {
            time: stats.balancer.time,
            regionCount: stats.balancer.regionCount
          },
          workers: Array.from(stats.worker.values())
        }
      });

      this.setState(state =>
        Object.assign(state, {
          progress: ((state.configLength - configs.length) * 100) / state.configLength,
          output: JSON.stringify(output),
          currentConfig: c
        })
      );
      this.runConfig(output, configs);
    });
  }
}

const RenderConfig = (props: Config) => {
  return (
    <table>
      <tbody>
        <tr>
          <td>balancer:</td>
          <td>
            <pre className="config">{props.balancer}</pre>
          </td>
        </tr>
        <tr>
          <td>implementation:</td>
          <td>
            <pre className="config">{props.impl}</pre>
          </td>
        </tr>
        <tr>
          <td>Point of Interest</td>
          <td>
            <pre className="config">{JSON.stringify(props.poi)}</pre>
          </td>
        </tr>
      </tbody>
    </table>
  );
};
