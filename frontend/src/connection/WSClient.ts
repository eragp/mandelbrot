import { RegionData, Regions } from "./ExchangeTypes";
import { groupRegions, RegionGroup } from "../misc/RegionGroup";

const url = "ws://localhost:9002";

export default class WebSocketClient {
  private regionCallback: Array<((data: RegionGroup[]) => void)> = [];
  private workerCallback: Array<((data: RegionData) => void)> = [];
  private regionRequests: string[];
  private socket: WebSocket;

  private retRegions: number = 0;
  private currRegion: number = 1;

  constructor() {
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
          // Notify regionData/worker observers
          console.log("# " + this.currRegion++ + " of " + this.retRegions);
          workerCallback.forEach(call => {
            call(<RegionData>msg);
          });
          break;
        case "region":
          // Notify region subdivision listeners
          this.currRegion = 1;
          this.retRegions = (<Regions>msg).regions.length;
          regionCallback.forEach(call => call(groupRegions(<Regions>msg)));
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

  public sendRequest(request: {}) {
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
  public registerRegion(fun: (data: RegionGroup[]) => any) {
    this.registerCallback(this.regionCallback, fun);
  }

  /**
   * Registers a callback to call when the region data is returned
   */
  public registerRegionData(fun: (data: RegionData) => any) {
    this.registerCallback(this.workerCallback, fun);
  }

  /**
   * Registers an observer to a list
   */
  private registerCallback(list: any[], fun: (data: any) => any) {
    let promise: any;
    const render = (data: any) => {
      promise = new Promise((resolve, error) => {
        try {
          resolve(fun(data));
        } catch (err) {
          error(err);
        }
      });
    };
    list.push(render);
    return promise;
  }
}
