interface Worker {
  rank: number;
  computationTime: number;
  mpiTime: number;
  networkTime: number;
  drawTime: number;
}

interface Stats {
  worker: Map<number, Worker>;
  draw: Map<string, number>;
}

export class StatsCollector {
  public data: Stats;
  private waiting: number;

  private doneCallbacks: Array<((data: Stats) => any)>;

  constructor() {
    this.data = {
      worker: new Map(),
      draw: new Map()
    };
    this.waiting = 0;

    this.doneCallbacks = [];
  }

  public registerDone(fun: (data: Stats) => any) {
    this.doneCallbacks.push(fun);
  }

  /**
   *
   * @param id
   * @param time in us
   */
  public setNetworkTiming(rank: number, time: number) {
    let w = this.data.worker.get(rank);
    if (!w) {
      w = {
        rank,
        computationTime: -1,
        mpiTime: -1,
        networkTime: -1,
        drawTime: -1
      };
      this.data.worker.set(rank, w);
    }
    w.networkTime = time;
  }

  public incWaiting() {
    this.waiting++;
  }

  /**
   *
   * @param id
   * @param time in us
   */
  public setDrawTiming(id: string, time: number) {
    this.data.draw.set(id, time);
    this.waiting--;
    if (this.waiting === 0) this.done();
  }

  private done() {
    console.log("done rendering tiles");
    this.doneCallbacks.forEach(fn => fn(this.data));

    this.data.worker.clear();
    this.data.draw.clear();
  }
}
