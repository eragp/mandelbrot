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
            numWorkers: 1,
            active: [false],
            // The computation time in microseconds
            progress: [1]
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
                    },
                    intersect: false
                },
                title: {
                    display: true,
                    position: 'bottom',
                    text: ["Total node computation time:", "0 µs"]
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
            this.chartState.active[workerID - 1] = false;
            // TODO insert correct µs time in node value

            this.updateChart(0);
        });

        this.websocketClient.registerRegion(data => {
            // Stop redrawing
            this.stopNodeProgress();
            // Reset node progress
            const nworkers = data.regionCount;
            const active = new Array(nworkers);
            const progress = new Array(nworkers);

            let animationDuration = 750;

            for (let i = 0; i < nworkers; i++) {
                active[i] = true;
                progress[i] = animationDuration * 1000;
            }
            this.chartState = {
                numWorkers: nworkers,
                active: active,
                progress: progress
            };
            this.updateChart(animationDuration);
            // Start redrawing as soon as animation has finished
            setTimeout(() => {
                this.initNodeProgress()
            }, animationDuration);
        });
    }

    render() {
        return (<div className="nodeProgress"><canvas id="nodeProgress"></canvas></div>);
    }

    updateChart(animationDuration) {
        const progress = this.chartState.progress;
        const labels = [];
        const colorSet = [];
        for (let i = 0; i < progress.length; i++) {
            labels.push("Worker " + i);
            colorSet.push(this.props.workerContext.getWorkerColor(i))
        }
        
        const data = {
            labels: labels,
            datasets: [{
                data: progress,
                backgroundColor: colorSet

            }]
            // TODO include nice colors
        };
        this.chart.data = data;

        let computationTime = 0;
        for (let i = 0; i < progress.length; i++) {
            computationTime += progress[i];
        }
        this.chart.options.title.text[1] = computationTime + " µs";

        this.chart.update(animationDuration);
    }

    /**
     * Start redrawing the current node computation time every 50 milliseconds
     */
    initNodeProgress() {
        // Interval in milliseconds
        const interval = 50;
        this.interval = setInterval(
            state => {
                let update = false;
                for (let i = 0; i < state.progress.length; i++) {
                    if (state.active[i]) {
                        state.progress[i] += interval * 1000;
                        update = true;
                    }
                }
                if (update) {
                    // Animation duration of 0 for fluent redrawing
                    this.updateChart(0);
                }
            },
            interval,
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