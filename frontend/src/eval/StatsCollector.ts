import { registerCallback } from "../misc/registerCallback";

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
  private data: Stats;
  private waiting: number;

  private doneCallbacks: Array<((data: Stats) => any)>;

  constructor() {
    this.data = {
      worker: new Map()
    };
    this.waiting = 0;

    this.doneCallbacks = [];
  }

  /**
   *
   * @param fun callback to be executed ONCE when stats
   * collection is done for the current region
   */
  public onDone(fun: (data: Stats) => any) {
    return registerCallback(this.doneCallbacks, fun);
  }

  /**
   *
   * @param id
   * @param time in us
   */
  public setComputationTime(rank: number, time: number) {
    this.addRank(rank).computationTime = Math.floor(time);
  }

  /**
   *
   * @param rank
   * @param time in us
   */
  public setDrawTime(rank: number, time: number) {
    this.addRank(rank).drawTime += Math.floor(time);
  }

  public getWaiting() {
    return this.waiting;
  }

  public setWaiting(w: number) {
    if ((this.waiting = w) === 0) {
      this.done();
    }
  }

  public done() {
    const cb = this.doneCallbacks;
    this.doneCallbacks = [];
    cb.forEach(fn => fn(this.data));
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
