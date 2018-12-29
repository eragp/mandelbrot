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
let configs: Config[] = [];
export const startTour = (
  stats: StatsCollector,
  viewCenter: ViewCenterObservable,
  balancer: BalancerObservable,
  impl: ImplementationObservable
) => {
  const tour = require("./config.json") as Tour;

  const out: Output = { config: tour, datapoints: [] };
  for (const balancer of tour.balancers) {
    for (const impl of tour.implementations) {
      for (const poi of tour.pois) {
        configs.push({ balancer, impl, poi });
      }
    }
  }
  // console.log("set center ", poi);
  // viewCenter.set(new Point3D(poi.real, poi.imag, poi.zoom));

  const cont = (output: Output) => {
    if (configs.length === 0) {
      console.log(output);
      return;
    }
    const c = configs.pop() as Config;
    configs = configs.slice(0, configs.length - 1);

    balancer.set(c.balancer);
    impl.set(c.impl);
    let p = new Point3D(c.poi.real, c.poi.imag, c.poi.zoom);
    viewCenter.set(p);

    stats.registerDone(stats => {
      out.datapoints.push({
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
      cont(out);
    });
  };
  cont(out);
  // const str = JSON.stringify(out);
};
