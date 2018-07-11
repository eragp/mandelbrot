import React, { Component } from 'react';
import { Doughnut } from 'react-chartjs-2';

export default class extends Component {

    constructor(props){
        super(props);
        this.websocketClient = props.wsclient;
    }

    componentWillMount() {
        this.setState({
            numWorkers: 0,
            active: [false],
            // The computation time in microseconds
            progress: [1]
          });
    }

    componentDidMount() {
        console.log(this.refs.chart.chart_instance);
        // Interval in milliseconds
        let interval = 500;
        let _this = this;
        this.interval = setInterval(
            () => {      
                _this.setState((oldState) => {
                    for(let i = 0; i < oldState.progress.length; i++){
                        if(oldState.active[i]){
                            oldState.progress[i] += interval * 1000;
                        }
                    }
                    return oldState;
                });
            },
            interval
        );

        // register workers at websocket client
        // so that they are set inactive when the first tile/region
        // by them comes in
        this.websocketClient.registerWorker((data) => {
            // Stop corresponding worker progress bar
            // assume that regionData is passed here
            let workerID = data.workerInfo.rank;
            this.setState((oldState) => {
                // Pay attention here that ranks begin from 1 as long as the host does not send data on his own
                oldState.active[workerID-1] = false;
                // TODO insert correct µs time in node value
                return oldState;
            });
        });
        
        this.websocketClient.registerRegion((data) => {
            // Draw node progress
            let nworkers = data.regionCount;
            let active = new Array(nworkers);
            let progress = new Array(nworkers);
            for(var i = 0; i < nworkers; i++){
                active[i] = true;
                progress[i] = 0;
            }
            this.setState({
                numWorkers: nworkers,
                active: active,
                progress: progress
            });
        });
    }

    render(){
        let progress = this.state.progress;
        let labels = [];
        for(let i = 0; i < progress.length; i++){
            labels.push("Worker " + i);
        }
        let data = {
            labels: labels,
            datasets: [{
                data: progress
            }]
        };
        let options = {
            responsive: false
        };
        console.log(data);
        this.reference = null;
        return (<div><Doughnut ref='chart' data={data} options={options}/></div>);
    }
    
}