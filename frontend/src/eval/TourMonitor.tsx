import * as React from "react";
import {
  BalancerObservable,
  ImplementationObservable,
  ViewCenterObservable,
  WorkerObservable
} from "../misc/Observable";
import { Point3D } from "../misc/Point";
import { StatsCollector } from "./StatsCollector";
import { Output, Setting, PoI, Tour } from "./ConfigTypes";

import "./TourMonitor.css";

interface Config {
  setting: Setting;
  poi: PoI;
}

interface TourMonitorProps {
  stats: StatsCollector;
  viewCenter: ViewCenterObservable;
  balancer: BalancerObservable;
  impl: ImplementationObservable;
  workerCount: WorkerObservable;
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
    const config = require("./configSIMD.json") as Tour;

    // generate config combinations
    this.configs = [];
    for (const setting of config.settings) {
      for (const poi of config.pois) {
        this.configs.push({ setting, poi });
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
    const c = configs[0] as Config;
    console.log("Tour: Changing config to: ", c);

    // only notify the observable that has changed once
    const oldBl = this.props.balancer.get();
    const oldImp = this.props.impl.get();
    const oldCenter = this.props.viewCenter.get();

    this.props.balancer.setNoNotify(c.setting.balancer);
    this.props.impl.setNoNotify(c.setting.implementation);
    const pt = new Point3D(c.poi.real, c.poi.imag, c.poi.zoom);
    this.props.viewCenter.setNoNotify(pt);
    this.props.workerCount.setNoNotify(c.setting.nodes);

    if (oldBl !== c.setting.balancer) {
      this.props.balancer.notify();
    } else if (oldImp !== c.setting.implementation) {
      this.props.impl.notify();
    } else if (!oldCenter.equals(pt)) {
      this.props.viewCenter.notify();
    } else {
      this.props.workerCount.notify();
    }

    this.props.stats.onDone(stats => {
      output.datapoints.push({
        setting: c.setting,
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
      console.log(JSON.stringify(output));
      this.runConfig(output, configs.slice(1));
    });
  }
}

const RenderConfig = (props: Config) => {
  return (
    <table>
      <tbody>
        {Tr("Balancer:", props.setting.balancer)}
        {Tr("Implementation:", props.setting.implementation)}
        {Tr("Nodes:", props.setting.nodes)}
        {Tr("Point of Interest:", props.poi)}
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
