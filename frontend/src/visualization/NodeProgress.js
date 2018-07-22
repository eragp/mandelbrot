import React, { Component } from 'react';
import { Chart } from 'chart.js';

export default class extends Component {

    constructor(props){
        super(props);
        this.websocketClient = props.wsclient;
        this.chartState;
    }

    componentWillMount() {
        this.chartState ={
            numWorkers: 0,
            active: [false],
            // The computation time in microseconds
            progress: [1]
          };
    }

    componentDidMount() {
        
        var ctx = document.getElementById("nodeProgress");
        this.chart = new Chart(ctx, {
            type: 'doughnut',
            data: [],
            options: {
                responsive: false
            }
        });

        // Interval in milliseconds
        let interval = 250;
        let _this = this;
        this.interval = setInterval(
            () => { 
                let state = _this.chartState;
                for(let i = 0; i < state.progress.length; i++){
                    if(state.active[i]){
                        state.progress[i] += interval * 1000;
                    }
                }
                _this.updateChart();
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
            // Pay attention here that ranks begin from 1 as long as the host does not send data on his own
            _this.chartState.active[workerID-1] = false;
            // TODO insert correct µs time in node value

            _this.updateChart();
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
            _this.chartState = {
                numWorkers: nworkers,
                active: active,
                progress: progress
            };
            _this.updateChart();
        });
    }

    render(){
        return (<canvas id={"nodeProgress"}></canvas>);
    }

    updateChart(){
        let progress = this.chartState.progress;
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
        this.chart.data = data;
        this.chart.update(0);
    }
    
}