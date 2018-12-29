import { registerCallback } from "../misc/registerCallback";
import { ADDRCONFIG } from "dns";

interface Worker {
  rank: number;
  computationTime: number;
  mpiTime: number;
  networkTime: number;
  drawTime: number;
}

interface Stats {
  worker: Map<number, Worker>;
}

export class StatsCollector {
  public data: Stats;
  private waiting: number;

  private doneCallbacks: Array<((data: Stats) => any)>;

  constructor() {
    this.data = {
      worker: new Map()
    };
    this.waiting = 0;

    this.doneCallbacks = [];
  }

  public registerDone(fun: (data: Stats) => any) {
    return registerCallback(this.doneCallbacks, fun);
  }

  /**
   *
   * @param id
   * @param time in us
   */
  public setNetworkTiming(rank: number, time: number) {
    let w = this.addRank(rank);
    w.networkTime = time;
  }

  /**
   *
   * @param rank
   * @param time in us
   */
  public setDrawTiming(rank: number, time: number) {
    let w = this.addRank(rank);
    w.drawTime += time;
  }

  public getWaiting() {
    return this.waiting;
  }

  public setWaiting(w: number) {
    this.waiting = w;
    if (this.waiting === 0) {
      this.done();
    }
  }
  
  public done() {
    console.log("done rendering tiles");
    this.doneCallbacks.forEach(fn => fn(this.data));

    this.data.worker.clear();
    this.waiting = 0;
  }

  private addRank(rank: number) {
    let w = this.data.worker.get(rank);
    if (!w) {
      w = {
        rank,
        computationTime: 0,
        mpiTime: 0,
        networkTime: 0,
        drawTime: 0
      };
      this.data.worker.set(rank, w);
    }
    return w;
  }

}
