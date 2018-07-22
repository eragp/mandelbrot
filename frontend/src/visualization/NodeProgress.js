import React, { Component } from 'react';
import { Chart } from 'chart.js';
import './NodeProgress.css';

/**
 * Colors for the workers
 * TODO replace with nice colorset (i.e. theme)
 */
const colorSet =  [
    '#4661EE',
    '#EC5657',
    '#1BCDD1',
    '#8FAABB',
    '#B08BEB',
    '#3EA0DD',
    '#F5A52A',
    '#23BFAA',
    '#FAA586',
    '#EB8CC6',
];

export default class extends Component {

    constructor(props){
        super(props);
        this.websocketClient = props.wsclient;
        this.chartState;
    }

    componentWillMount() {
        this.chartState ={
            numWorkers: 1,
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
                responsive: false,
                tooltips: {
                    callbacks: {
                        label: function(tooltipItem, data) {
                            var label = data.labels[tooltipItem.index];
        
                            if (label) {
                                label += ': ';
                            } else {
                                label = '';
                            }
                            label += data.datasets[tooltipItem.datasetIndex].data[tooltipItem.index] + ' µs';
                            return label;
                        }
                    }
                }
            }
        });
        this.updateChart();

        this.initNodeProgress();

        let _this = this;
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

            _this.updateChart(0);
        });
        
        this.websocketClient.registerRegion((data) => {
            // Stop redrawing
            _this.stopNodeProgress();
            // Reset node progress
            let nworkers = data.regionCount;
            let active = new Array(nworkers);
            let progress = new Array(nworkers);

            let animationDuration = 1000;

            for(var i = 0; i < nworkers; i++){
                active[i] = true;
                progress[i] = animationDuration * 1000;
            }
            _this.chartState = {
                numWorkers: nworkers,
                active: active,
                progress: progress
            };
            _this.updateChart(animationDuration);
            // Start redrawing as soon as animation has finished
            setTimeout(() => {_this.initNodeProgress()}, animationDuration);
        });
    }

    render(){
        return (<canvas id={"nodeProgress"}></canvas>);
    }

    updateChart(animationDuration){
        let progress = this.chartState.progress;
        let labels = [];
        for(let i = 0; i < progress.length; i++){
            labels.push("Worker " + i);
        }
        let data = {
            labels: labels,
            datasets: [{
                data: progress,
                backgroundColor: colorSet
               
            }]
            // TODO include nice colors
        };
        this.chart.data = data;
        this.chart.update(animationDuration);
    }

    /**
     * Start redrawing the current node computation time every 50 milliseconds
     */
    initNodeProgress(){
        // Interval in milliseconds
        let interval = 50;
        let _this = this;
        this.interval = setInterval(
            (state) => { 
                let update = false;
                for(let i = 0; i < state.progress.length; i++){
                    if(state.active[i]){
                        state.progress[i] += interval * 1000;
                        update = true;
                    }
                }
                if(update){
                    // Animation duration of 0 for fluent redrawing
                    _this.updateChart(0);
                }
            },
            interval,
            this.chartState
        );
    }

    /**
     * Stop redrawing the node progress every 50 milliseconds
     */
    stopNodeProgress(){
        clearInterval(this.interval);
    }
    
}