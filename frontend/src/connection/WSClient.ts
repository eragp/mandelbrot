const url = "ws://localhost:9002";

export default class WebSocketClient {
  private regionCallback: Array<((data: Regions) => void)> = [];
  private workerCallback: Array<((data: RegionData) => void)> = [];
  private regionRequests: string[];
  private socket: WebSocket;

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
      s.onopen = () => setTimeout(1, () => s.close());
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
          workerCallback.forEach(callback => callback(<RegionData>msg));
          break;
        case "region":
          // Notify region subdivision listeners
          regionCallback.forEach(callback => callback(<Regions>msg));
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
  public registerRegion(fun: (data: Regions) => any) {
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

export interface RegionData {
  data: number[];
  type: string;
  workerInfo: WorkerInfo;
}

export interface Regions {
  type: string;
  regions: WorkerInfo[];
}

interface WorkerInfo {
  rank: number;
  computationTime: number;
  region: Region;
}

export interface Request {
  balancer: string;
  guaranteedDivisor: number;
  width: number;
  height: number;
  minImag: number;
  maxImag: number;
  minReal: number;
  maxReal: number;
  validation: number;
  maxIteration: number;
}

export interface Region extends Request {
  hOffset: number;
  vOffset: number;
}
