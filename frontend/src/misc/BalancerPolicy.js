/**
 * All available balancers
 */
export const BALANCERS = [
    'naive',
    'column',
    'prediction'
    // To be continued...
]

/**
 * Object returning the currently chosen balancing strategy
 */
export default class BalancerPolicy {

    constructor() {
        this.balancer = 'naive';
        this.callbacks = [];
    }

    getBalancer() {
        return this.balancer;
    }

    setBalancer(balancer) {
        if (!BALANCERS.includes(balancer)) {
            throw "Invalid balancer type: " + balancer;
        }
        this.balancer = balancer;
        this.updateAll();
    }

    updateAll() {
        this.callbacks.forEach(cb => cb(this.balancer))
    }

    subscribe(callback) {
        let promise;
        const fun = data => {
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

}