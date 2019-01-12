import { registerCallback } from "../misc/registerCallback";
import WSClient from "../connection/WSClient";

interface Worker {
  rank: number;
  computationTime: number;
  mpiTime: number;
  drawTime: number;
}
interface Balancer {
  time: number;
  regionCount: number;
}

interface Stats {
  worker: Map<number, Worker>;
  balancer: Balancer;
}

export class StatsCollector {
  private data: Stats;
  private waiting: number;

  private doneCallbacks: Array<((data: Stats) => any)>;

  constructor(wsclient: WSClient) {
    this.data = {
      worker: new Map(),
      balancer: { regionCount: 0, time: 0 }
    };
    this.waiting = 0;

    this.doneCallbacks = [];

    // Register for regionData and region subdivisions
    wsclient.registerRegionRaw(r => {
      this.setWaiting(r.regionCount);
      this.setBalancerTime(r.regionCount, r.balancerTime);
    });
    wsclient.registerRegionData(r => {
      this.setComputationTime(r.workerInfo.rank, r.workerInfo.computationTime);
      this.setMpiTime(r.workerInfo.rank, r.workerInfo.mpiTime);
      this.setWaiting(this.getWaiting() - 1);
    });
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
   * @param id
   * @param time in us
   */
  public setMpiTime(rank: number, time: number) {
    this.addRank(rank).mpiTime = Math.floor(time);
  }

  public setBalancerTime(regionCount: number, time: number) {
    this.data.balancer = { regionCount, time };
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

    // reset this object
    this.data.worker.clear();
    this.data.balancer = { regionCount: 0, time: 0 };
    this.waiting = 0;
  }

  private addRank(rank: number) {
    let w = this.data.worker.get(rank);
    if (!w) {
      w = {
        rank,
        computationTime: 0,
        mpiTime: 0,
        drawTime: 0
      };
      this.data.worker.set(rank, w);
    }
    return w;
  }
}
