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

export const startTour = (
  stats: StatsCollector,
  viewCenter: ViewCenterObservable,
  balancer: BalancerObservable,
  impl: ImplementationObservable
) => {
  const config = require("./config.json") as Tour;

  let cfgs: Config[] = [];
  const out: Output = { config, datapoints: [] };
  for (const balancer of config.balancers) {
    for (const impl of config.implementations) {
      for (const poi of config.pois) {
        cfgs.push({ balancer, impl, poi });
      }
    }
  }
  console.log("running configs:", cfgs);

  // set size of leaflet container
  const map = document.getElementById("viewer") as HTMLElement;
  map.style.width = `${config.screen.width}px`;
  map.style.height = `${config.screen.height}px`;

  const runConfig = (output: Output, configs: Config[]) => {
    // tour is done
    if (configs.length === 0) {
      // reset leaflet container to original size
      map.style.width = "";
      map.style.height = "";

      console.log("Tour Stats:");
      console.log(JSON.stringify(output));
      return;
    }
    const c = configs.pop() as Config;
    console.log("Tour: Changing config to: ", c);

    const oldBl = balancer.get();
    const oldImp = impl.get();

    balancer.setNoNotify(c.balancer);
    impl.setNoNotify(c.impl);
    viewCenter.setNoNotify(new Point3D(c.poi.real, c.poi.imag, c.poi.zoom));
    if (oldBl !== c.balancer) {
      balancer.notify();
    } else if (oldImp !== c.impl) {
      impl.notify();
    } else {
      viewCenter.notify();
    }

    stats.onDone(stats => {
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
      runConfig(output, configs);
    });
  };

  // reset stats
  stats.done();
  runConfig(out, cfgs);
};
