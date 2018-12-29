interface Stats {
  network: Map<number, number>;
  draw: Map<string, number>;
}

// enum StatTypes {
//   net = "NET",
//   balancer = "BAL",
//   mpi = "MPI",
//   draw = "DRAW"
// }

export class StatsCollector {
  private data: Stats;
  private waiting: number;

  private doneCallbacks: Array<((data: Stats) => any)>;

  constructor() {
    this.data = {
      network: new Map(),
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
  public setNetworkTiming(id: number, time: number) {
    this.data.network.set(id, time);
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
    this.doneCallbacks.forEach(fn => fn(this.data));

    this.data.network.clear();
    this.data.draw.clear();
  }
}
