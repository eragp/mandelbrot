import WebSocketClient, { WS } from "../connection/WSClient";
import { RegionData } from "../connection/ExchangeTypes";
import { RegionGroup } from "../misc/RegionGroup";
import {
  BalancerObservable,
  GroupObservable,
  ImplementationObservable,
  ViewCenterObservable
} from "../misc/Observable";
import { setURLParams } from "../misc/URLParams";
import { Point3D } from "../misc/Point";
import { registerCallback } from "../misc/registerCallback";
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
  data: Datapoint[];
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
  id: number;
  computationTime: number;
  mpiTime: number;
  networkTime: number;
  drawTime: number;
}

const run: Run = {
  balancer: "naive",
  implementation: "default",
  data: [
    {
      poi: { real: 0, imag: 0, zoom: 0 },
      balancer: {
        time: 0,
        emptyRegions: 0
      },
      workers: [
        {
          id: 0,
          computationTime: 1,
          mpiTime: 1,
          networkTime: 1,
          drawTime: 1
        }
      ]
    }
  ]
};

export const startTour = (
  stats: StatsCollector,
  viewCenter: ViewCenterObservable,
  balancer: BalancerObservable,
  impl: ImplementationObservable
) => {
  const tour = require("./config.json") as Tour;

  const out: Output = { config: tour, datapoints: [] };
  for (const balancerType of tour.balancers) {
    balancer.set(balancerType);
    for (const implType of tour.implementations) {
      impl.set(implType);
      for (const poi of tour.pois) {
        console.log("set center ", poi);
        viewCenter.set(new Point3D(poi.real, poi.imag, poi.zoom));
      }
    }
  }

  console.log(out);
  // const str = JSON.stringify(out);
};
