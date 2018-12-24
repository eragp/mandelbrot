import { Balancers, Implementations, ColorSet } from "../Constants";

class Observable<T> {
  private data: T;
  private callbacks: Array<((data: T) => void)>;
  private possibleValues: T[] | null;

  constructor(defaultChoice: T, values: T[] | null) {
    this.data = defaultChoice;
    this.callbacks = [];
    this.possibleValues = values;
  }

  get() {
    return this.data;
  }

  set(data: T) {
    if (this.possibleValues && !this.possibleValues.includes(data)) {
      // console.error(`${data} not in ${this.possibleValues}`);
      return false;
    } else if (this.data !== data) {
      this.data = data;
      this.notify();
      return true;
    } else {
      return false;
    }
  }

  subscribe(cb: ((data: T) => void)) {
    this.callbacks.push(cb);
  }

  notify() {
    this.callbacks.forEach(fn => fn(this.data));
  }
}

export class BalancerObservable extends Observable<string> {
  constructor() {
    super(Balancers[0].key, Balancers.map(b => b.key));
  }
}

export class ImplementationObservable extends Observable<string> {
  constructor() {
    super(Implementations[0].key, Implementations.map(i => i.key));
  }
}

export class GroupObservable extends Observable<number | undefined> {
  constructor() {
    super(1, null);
  }

  getColor(nodeID: number) {
    return ColorSet[nodeID % ColorSet.length];
  }

  set(nodeID: number | undefined) {
    return super.set(nodeID);
  }
}
