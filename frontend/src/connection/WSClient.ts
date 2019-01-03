import { RegionData, Regions, Request } from "./ExchangeTypes";
import { groupRegions, RegionGroup } from "../misc/RegionGroup";
import { registerCallback } from "../misc/registerCallback";
import { StatsCollector } from "../eval/StatsCollector";

const url = "ws://localhost:9002";
export interface WS {
  sendRequest(request: any): void;
  registerRegion(fun: (groups: RegionGroup[]) => any): Promise<any>;
  registerRegionData(fun: (groups: RegionData) => any): Promise<any>;
  close(): void;
}

export default class WebSocketClient implements WS {
  private regionCallback: Array<((data: RegionGroup[]) => void)> = [];
  private workerCallback: Array<((data: RegionData) => void)> = [];
  private regionRequests: string[];
  private socket: WebSocket;

  constructor(stats?: StatsCollector) {
    /**
     * Callbacks for any methods interested in new region subdivisions or regionData (=result of one worker)
     */
    const regionCallback = this.regionCallback;
    const workerCallback = this.workerCallback;

    // Web Socket setup
    // Necessary due to a backend bug
    // TODO: remove this as it's a dirty hack
    {
      let s = new WebSocket(url);
      s.onopen = () => setTimeout(() => s.close(), 1);
    }

    let socket = new WebSocket(url);
    // Buffer of requests to be sent when the socket connects
    this.regionRequests = [];
    socket.onopen = () => {
      this.regionRequests.forEach(message => socket.send(message));
    };

    // Restart the socket connection on close (optional, as the frontend does not get a notification
    // that the connection failed on the first try)
    socket.onclose = () => {
      setTimeout(() => {
        socket = new WebSocket(url);
      }, 30000);
      // TODO: maybe in more beautiful, less annoying
      //alert('Websocket connection failed, reconnecting in 30s')
    };

    socket.onmessage = event => {
      const msg = JSON.parse(event.data);
      switch (msg.type) {
        case "regionData":
          {
            const r = msg as RegionData;
            // collect timing data
            if (stats) {
              stats.setComputationTime(r.workerInfo.rank, r.workerInfo.computationTime);
            }
            // Notify regionData/worker observers
            workerCallback.forEach(fn => fn(r));
            if (stats) {
              stats.setWaiting(stats.getWaiting() - 1);
            }
          }
          break;
        case "region":
          {
            const r = msg as Regions;
            if (stats) {
              console.log("region answer");
              stats.setWaiting(r.regionCount);
              stats.setBalancerTime(r.regionCount, r.balancerTime);
            }
            const g = groupRegions(r.regions);
            // Notify region subdivision listeners
            regionCallback.forEach(fn => fn(g));
          }
          break;
        default:
      }
    };
    this.socket = socket;
  }

  public close() {
    console.log("closing the WS connection");
    this.socket.close();
  }

  public sendRequest(request: Request) {
    const message = JSON.stringify(request);
    if (this.socket.readyState === this.socket.OPEN) {
      this.socket.send(message);
    } else {
      this.regionRequests.push(message);
    }
  }
  /**
   * Registers a callback to call when the region subdivision is returned
   */
  public registerRegion(fun: (groups: RegionGroup[]) => any) {
    return registerCallback(this.regionCallback, fun);
  }

  /**
   * Registers a callback to call when the region data is returned
   */
  public registerRegionData(fun: (data: RegionData) => any) {
    return registerCallback(this.workerCallback, fun);
  }
}
