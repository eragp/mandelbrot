import { Balancers } from "../Constants";
/**
 * All available balancers
 */
export const BALANCERS = Balancers.map(b => b.key);

/**
 * Object returning the currently chosen balancing strategy
 */
export default class BalancerPolicy {

  private balancer: string;
  private callbacks: Array<(s: string) => void>;

  constructor() {
    this.balancer = "naive";
    this.callbacks = [];
  }

  public getBalancer() {
    return this.balancer;
  }

  public setBalancer(balancer: string) {
    if (!BALANCERS.includes(balancer)) {
      console.error(`Invalid balancer type: ${balancer}`);
      return;
    }
    this.balancer = balancer;
    this._updateAll();
  }

  public subscribe(callback: (s: string) => any): Promise<{}> | undefined {
    let promise;
    const fun = (data: string) => {
      promise = new Promise((resolve, error) => {
        try {
          resolve(callback(data));
        } catch (err) {
          error(err);
        }
      });
    };
    this.callbacks.push(fun);
    return promise;
  }

  private _updateAll() {
    this.callbacks.forEach(cb => cb(this.balancer));
  }

}
