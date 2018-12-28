import WebSocketClient, { WS, registerCallback } from "../connection/WSClient";
import { RegionData } from "../connection/ExchangeTypes";
import { RegionGroup } from "../misc/RegionGroup";
import { BalancerObservable, GroupObservable, ImplementationObservable } from "../misc/Observable";
import { setURLParams } from "../misc/URLParams";
import { Point3D } from "../misc/Point";

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

const data: Run = {
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

const defaultTour: Tour = {
  screen: {
    width: 1920,
    height: 1080
  },
  balancers: ["naive", "prediction"],
  implementations: ["default", "SIMD64"],
  nodeCount: 10,
  description: "",
  pois: [{ real: 0.0, imag: 0.0, zoom: 0 }, { real: 0.5, imag: 0.0, zoom: 0 }]
};

export const startTour = (
  wsClient: WebSocketClient,
  balancer: BalancerObservable,
  group: GroupObservable,
  impl: ImplementationObservable
) => {
  const client = new WSDecorator(wsClient);
  const tour = defaultTour;

  let b: BalancerTime = null;
  client.registerRegion(groups => {
    b = { time: 1234, emptyRegions: groups.length };
  });

  let w: WorkerTime[] = [];
  client.registerRegionData(data => {
    const wi = data.workerInfo;
    w.push({
      id: wi.rank,
      computationTime: wi.computationTime,
      mpiTime: wi.computationTime,
      networkTime: wi.computationTime,
      drawTime: -1 // cannot determine draw time here
    });
  });

  const out: Output = { config: tour, datapoints: [] };
  for (const balancerType of tour.balancers) {
    balancer.set(balancerType);
    for (const implType of tour.implementations) {
      impl.set(implType);
      for (const poi of tour.pois) {
        const wait = async () => {
          setURLParams(new Point3D(poi.real, poi.imag, poi.zoom));
          await client.registerDone(() => null);
        };
        wait();
        out.datapoints.push({
          balancer: balancerType,
          implementation: implType,
          data: [
            {
              poi,
              balancer: b,
              workers: w
            }
          ]
        });
      }
    }
  }

  const str = JSON.stringify(out);
  console.log(str);
};

class WSDecorator implements WS {
  private doneCallback: Array<() => void> = [];
  private waiting = 0;
  private ws: WebSocketClient;

  constructor(ws: WebSocketClient) {
    this.ws = ws;
    ws.registerRegion(groups => {
      this.waiting = groups.map(g => g.getLeafs().length).reduce((acc, curr) => acc + curr);
    });
    ws.registerRegionData(() => {
      this.waiting--;
      if (this.waiting === 0) {
        this.doneCallback.forEach(cb => cb());
      }
    });
  }

  public registerDone(fun: () => void) {
    return registerCallback(this.doneCallback, fun);
  }

  public sendRequest(request: any): void {
    return this.ws.sendRequest(request);
  }
  public registerRegion(fun: (groups: RegionGroup[]) => any): Promise<any> {
    return this.ws.registerRegion(fun);
  }
  public registerRegionData(fun: (groups: RegionData) => any): Promise<any> {
    return this.ws.registerRegionData(fun);
  }
  public close(): void {
    this.ws.close();
  }
}
