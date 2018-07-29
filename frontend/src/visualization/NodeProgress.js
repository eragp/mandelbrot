import React, {
    Component
} from 'react';
import {
    Chart
} from 'chart.js';
import WebSocketClient from '../connection/WSClient';
import PropTypes from 'prop-types';

import './NodeProgress.css';
import WorkerContext from '../misc/WorkerContext';

/**
 * Shows the computation time of invoked workers
 * Additional documentation on the type of used chart: https://www.chartjs.org/docs/latest/
 */
export default class NodeProgress extends Component {

    constructor(props) {
        super(props);
        this.websocketClient = props.wsclient;
        this.chartState = {};

        this.chartState = {
            nodes: [0],
            active: new Map([[0, false]]),
            // The computation time in microseconds
            progress: new Map([[0, 1]])
        };
    }

    componentDidMount() {

        const ctx = document.getElementById("nodeProgress");
        const customLabel = (tooltipItem, data) => {
            let label = data.labels[tooltipItem.index];

            if (label) {
                label += ': ';
            } else {
                label = '';
            }
            label += data.datasets[tooltipItem.datasetIndex].data[tooltipItem.index] + ' µs';
            return label;
        }

        this.chart = new Chart(ctx, {
            type: 'doughnut',
            data: [],
            options: {
                responsive: true,
                maintainAspectRatio: false,
                tooltips: {
                    callbacks: {
                        label: customLabel
                    }
                },
                title: {
                    display: true,
                    position: 'bottom',
                    text: ["Total node computation time:", "0 µs"]
                },
                onHover: (event) => {
                    // change workercontext active worker on hover
                    const data = this.chart.getElementsAtEvent(event)[0];
                    if(data){
                        this.props.workerContext.setActiveWorker(data._index);
                        this._hoveredItem = data;
                    }
                    else if(this._hoveredItem){
                        this.props.workerContext.setActiveWorker(undefined);
                        this._hoveredItem = undefined;
                    }
                }
            }
        });
        this.updateChart();

        this.initNodeProgress();

        // register workers at websocket client
        // so that they are set inactive when the first tile/region
        // by them comes in
        this.websocketClient.registerWorker(data => {
            // Stop corresponding worker progress bar
            // assume that regionData is passed here
            const workerID = data.workerInfo.rank;
            // Pay attention here that ranks begin from 1 as long as the host does not send data on his own
            this.chartState.active.set(workerID, false);
            // insert correct µs time in node value
            this.chartState.progress.set(workerID, data.workerInfo.computationTime);
            this.updateChart(0);
        });

        this.websocketClient.registerRegion(data => {
            // Stop redrawing
            this.stopNodeProgress();
            // Reset node progress
            const nodes = [];
            const active = new Map();
            const progress = new Map();

            let animationDuration = 750;
            for(let region of data.regions){
                nodes.push(region.nodeID);
                active.set(region.nodeID, true);
                progress.set(region.nodeID, animationDuration * 1000);
            }
            this.chartState = {
                nodes: nodes,
                active: active,
                progress: progress
            };
            this.updateChart(animationDuration);
            // Start redrawing as soon as animation has finished
            setTimeout(() => {
                this.initNodeProgress()
            }, animationDuration);
        });

        // Highlight segement on active worker change
        // Inspired by https://github.com/chartjs/Chart.js/issues/1768
        this.props.workerContext.subscribe(activeWorker => {
            // Activate new tooltip if necessary
            if(activeWorker !== undefined){
                const workerIndex = this.chartState.nodes.indexOf(activeWorker);
                const activeSegment = this.chart.data.datasets[0]._meta[0].data[workerIndex];
                this.chart.tooltip.initialize();
                this.chart.tooltip._active = [activeSegment];
                this.chart.data.datasets[0]._meta[0].controller.setHoverStyle(activeSegment);
                this._hoveredSegment = activeSegment;
                
            } else {
                // Remove tooltip
                this.chart.data.datasets[0]._meta[0].controller.removeHoverStyle(this._hoveredSegment);
                this.chart.tooltip._active = [];
            }
            // Update chart
            this.chart.tooltip.update(true);
            this.chart.render(this.chart.options.hover.animationDuration, false);
                
        });
    }

    render() {
        return (<div className="nodeProgress"><canvas id="nodeProgress"></canvas></div>);
    }

    updateChart(animationDuration) {
        const labels = [];
        const values = [];
        const colorSet = [];
        // => Label/ value index is the index of the rank in the node array
        this.chartState.nodes.forEach((rank) => {
            labels.push("Worker " + rank);
            colorSet.push(this.props.workerContext.getWorkerColor(rank))
            values.push(this.chartState.progress.get(rank));
        });
        
        const data = {
            labels: labels,
            datasets: [{
                data: values,
                backgroundColor: colorSet

            }]
        };
        this.chart.data = data;

        let computationTime = 0;
        this.chartState.progress.forEach(value => {
            computationTime += value;
        });
        this.chart.options.title.text[1] = computationTime + " µs";

        this.chart.update(animationDuration);
    }

    /**
     * Start redrawing the current node computation time every 50 milliseconds
     */
    initNodeProgress() {
        // Interval in milliseconds
        const intervalRate = 50;
        this.interval = setInterval(
            state => {
                let update = false;
                state.progress.forEach((value, rank) => {
                    if (state.active.get(rank)) {
                        state.progress.set(rank, value + intervalRate * 1000);
                        update = true;
                    }
                });
                if (update) {
                    // Animation duration of 0 for fluent redrawing
                    this.updateChart(0);
                }
            },
            intervalRate,
            this.chartState
        );
    }

    /**
     * Stop redrawing the node progress every 50 milliseconds
     */
    stopNodeProgress() {
        clearInterval(this.interval);
    }

}

NodeProgress.propTypes = {
    wsclient: PropTypes.instanceOf(WebSocketClient),
    workerContext: PropTypes.instanceOf(WorkerContext)
};