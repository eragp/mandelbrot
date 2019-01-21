import {
  Balancers,
  Implementations,
  ColorSet,
  MaxIteration,
  NodeCount,
  PredictionAccuracy
} from "../Constants";
import { Point3D } from "./Point";

class Observable<T> {
  private data: T;
  private callbacks: Array<((data: T) => void)>;
  private possibleValues: T[] | null;

  constructor(defaultChoice: T, values: T[] | null) {
    this.data = defaultChoice;
    this.callbacks = [];
    this.possibleValues = values;
  }

  public get() {
    return this.data;
  }

  public set(data: T) {
    if (this.possibleValues && !this.possibleValues.includes(data)) {
      console.error(`${data} not in ${this.possibleValues}`);
      return false;
    } else if (this.data !== data) {
      this.data = data;
      this.notify();
      return true;
    } else {
      return false;
    }
  }

  public setNoNotify(data: T) {
    if (this.possibleValues && !this.possibleValues.includes(data)) {
      console.error(`${data} not in ${this.possibleValues}`);
      return false;
    } else if (this.data !== data) {
      this.data = data;
      return true;
    } else {
      return false;
    }
  }

  public subscribe(cb: ((data: T) => void)) {
    this.callbacks.push(cb);
  }

  public notify() {
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

export class ViewCenterObservable extends Observable<Point3D> {
  constructor() {
    super(null, null);
  }
}

export class WorkerObservable extends Observable<number> {
  constructor() {
    super(NodeCount, null);
  }
}

export class IterationObservable extends Observable<number> {
  constructor() {
    super(MaxIteration, null);
  }
}

export class PredAccObservable extends Observable<number> {
  constructor() {
    super(PredictionAccuracy, null);
  }
}

export class RunObservable extends Observable<number> {
  constructor() {
    super(1, null);
  }
}
