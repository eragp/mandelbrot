import {
  BalancerObservable,
  ImplementationObservable,
  ViewCenterObservable
} from "../misc/Observable";
import { Point3D } from "../misc/Point";
import { StatsCollector } from "./StatsCollector";

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
  emptyRegions: number;
}
interface WorkerTime {
  rank: number;
  computationTime: number;
  mpiTime: number;
  networkTime: number;
  drawTime: number;
}

interface Config {
  balancer: string;
  impl: string;
  poi: PoI;
}

export const startTour = (
  stats: StatsCollector,
  viewCenter: ViewCenterObservable,
  balancer: BalancerObservable,
  impl: ImplementationObservable
) => {
  const tour = require("./config.json") as Tour;

  let cfgs: Config[] = [];
  const out: Output = { config: tour, datapoints: [] };
  for (const balancer of tour.balancers) {
    for (const impl of tour.implementations) {
      for (const poi of tour.pois) {
        cfgs.push({ balancer, impl, poi });
      }
    }
  }
  console.log("running configs:", cfgs);

  window.resizeTo(tour.screen.width, tour.screen.height);
  const runConfig = (output: Output, configs: Config[]) => {
    if (configs.length === 0) {
      console.log("Tour Stats:");
      console.log(JSON.stringify(output));
      return;
    }
    const c = configs.pop() as Config;
    console.log("Tour: Changing config to: ", c);

    balancer.setNoNotify(c.balancer);
    impl.setNoNotify(c.impl);
    viewCenter.setNoNotify(new Point3D(c.poi.real, c.poi.imag, c.poi.zoom));
    viewCenter.notify();

    stats.onDone(stats => {
      output.datapoints.push({
        balancer: c.balancer,
        implementation: c.impl,
        data: {
          poi: c.poi,
          balancer: {
            time: 0,
            emptyRegions: 0
          },
          workers: Array.from(stats.worker.values())
        }
      });
      runConfig(output, configs);
    });
  };

  // reset stats
  stats.done();
  runConfig(out, cfgs);
};
