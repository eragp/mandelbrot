import { RegionData, Regions, WorkerInfo, workerInfoEquals, RegionRequest } from "./ExchangeTypes";
import { groupRegions, RegionGroup } from "./RegionGroup";
import { registerCallback } from "../misc/registerCallback";
import { WsUrl } from "../Constants";
import { setWSUrl } from "../misc/URLParams";

export interface WS {
  sendRequest(request: any): void;
  registerRegion(fun: (groups: RegionGroup[]) => any): Promise<any>;
  registerRegionData(fun: (groups: RegionData) => any): Promise<any>;
  close(): void;
}

export default class WebSocketClient implements WS {
  private regionCallback: Array<((data: RegionGroup[]) => void)> = [];
  private regionRawCallback: Array<((data: Regions) => void)> = [];
  private workerCallback: Array<((data: RegionData) => void)> = [];
  private regionRequests: string[];
  private socket: WebSocket;
  private currentRegions: WorkerInfo[];

  constructor(url?: string) {
    // if no url was provided use the default one
    if (!url) {
      url = WsUrl;
      setWSUrl(url);
    }
    console.log("connecting to ", url);
    /**
     * Callbacks for any methods interested in new Region subdivisions or regionData (=result of one worker)
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
            // do not filter empty RegionData
            const r = msg as RegionData;
            // filter answers for not current region
            if (!this.insideCurrentRegions(r.workerInfo)) {
              break;
            }
            // Notify regionData/worker observers
            workerCallback.forEach(fn => fn(r));
          }
          break;
        case "region":
          {
            const r = msg as Regions;
            // do not filter out empty regions
            this.regionRawCallback.forEach(cb => cb(r));
            const regions = r.regions;
            const g = groupRegions(regions);
            // Notify region subdivision listeners
            regionCallback.forEach(call => call(g));
            this.currentRegions = regions;
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

  public sendRequest(request: RegionRequest) {
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
  public registerRegionRaw(fun: (region: Regions) => any) {
    return registerCallback(this.regionRawCallback, fun);
  }

  /**
   * Registers a callback to call when the region data is returned
   */
  public registerRegionData(fun: (data: RegionData) => any) {
    return registerCallback(this.workerCallback, fun);
  }

  private insideCurrentRegions(data: WorkerInfo) {
    return this.currentRegions.some(w => workerInfoEquals(data, w));
  }
}
