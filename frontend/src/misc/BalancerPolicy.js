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
export default class {

    constructor(){
        this.balancer = 'naive';
    }

    getBalancer(){
        return this.balancer;
    }

    setBalancer(balancer){
        if(!BALANCERS.includes(balancer)){
            throw "Invalid balancer type: " + balancer; 
        }
        this.balancer = balancer;
    }

}