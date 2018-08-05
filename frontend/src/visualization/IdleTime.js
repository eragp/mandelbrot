import React, { Component } from 'react';
import { Chart } from 'chart.js';
import WebSocketClient from '../connection/WSClient';
import PropTypes from 'prop-types';

import './IdleTime.css';
import WorkerContext from '../misc/WorkerContext';

/**
 * Shows the computation time of invoked workers
 * Additional documentation on the type of used chart: https://www.chartjs.org/docs/latest/
 */
export default class IdleTime extends Component {
  constructor(props) {
    super(props);
    this.websocketClient = props.wsclient;

    this.chartState = {
      nodes: [0],
      active: new Map([[0, false]]),
      // The computation time in microseconds
      progress: new Map([[0, 1]])
    };
  }

  componentDidMount() {
    const ctx = document.getElementById('idleTime');
    const customLabel = (tooltipItem, data) => {
      let label = data.datasets[tooltipItem.datasetIndex].label;

      if (label) {
        label += ': ';
      } else {
        label = '';
      }
      label += data.datasets[tooltipItem.datasetIndex].data[tooltipItem.index];
      label += ' ms';
      return label;
    };

    this.chart = new Chart(ctx, {
      type: 'bar',
      data: [],
      options: {
        responsive: true,
        maintainAspectRatio: false,
        legend: {
          display: false
        },
        layout: {
          padding: {
            top: 10
          }
        },
        tooltips: {
          callbacks: {
            label: customLabel
          }
        },
        onHover: event => {
          // change workercontext active worker on hover
          const data = this.chart.getDatasetAtEvent(event)[0];
          if (data) {
            this.props.workerContext.setActiveWorker(
              this.chartState.nodes[data._datasetIndex]
            );
            this._hoveredItem = data;
          } else if (this._hoveredItem) {
            this.props.workerContext.setActiveWorker(undefined);
            this._hoveredItem = undefined;
          }
        },
        scales: {
          yAxes: [
            {
              type: 'linear',
              stacked: true,
              ticks: {
                beginAtZero: true,
                min: 0,
                suggestedMax: 10000
              },
              scaleLabel: {
                display: true,
                labelString: 'ms'
              }
            }
          ]
        }
      }
    });
    this.updateChart();

    this.initNodeProgress();

    // register workers at websocket client
    // so that they are set inactive when the first tile/region
    // by them comes in
    this.websocketClient.registerRegionData(data => {
      // Stop corresponding worker progress bar
      // assume that regionData is passed here
      // Pay attention here that ranks begin from 1 as long as the host does not send data on his own
      const workerID = data.workerInfo.rank;

      // Note that it is not active anymore
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

      for (let worker of data.regions) {
        nodes.push(worker.rank);
        active.set(worker.rank, true);
        progress.set(worker.rank, animationDuration * 1000);
      }
      this.chartState = {
        nodes: nodes,
        active: active,
        progress: progress
      };
      this.updateChart(animationDuration);
      // Start redrawing as soon as animation has finished
      setTimeout(() => {
        this.initNodeProgress();
      }, animationDuration);
    });

    // Highlight segement on active worker change
    // Inspired by https://github.com/chartjs/Chart.js/issues/1768
    this.props.workerContext.subscribe(activeWorker => {
      // Activate new tooltip if necessary
      if (activeWorker !== undefined) {
        const workerIndex = this.chartState.nodes.indexOf(activeWorker);
        const activeSegment = this.chart.data.datasets[workerIndex]._meta[0]
          .data[0];
        this.chart.tooltip.initialize();
        this.chart.tooltip._active = [activeSegment];
        this.chart.data.datasets[workerIndex]._meta[0].controller.setHoverStyle(
          activeSegment
        );
        this._hoveredSegment = activeSegment;
      } else {
        // Remove tooltip
        this.chart.data.datasets[0]._meta[0].controller.removeHoverStyle(
          this._hoveredSegment
        );
        this.chart.tooltip._active = [];
      }
      // Update chart
      this.chart.tooltip.update(true);
      this.chart.render(this.chart.options.hover.animationDuration, false);
    });
  }

  render() {
    return (
      <div className="idleTime">
        <canvas id="idleTime" />
      </div>
    );
  }

  updateChart(animationDuration) {
    const dataset = [];
    let maxComputationTime = 0;
    this.chartState.progress.forEach(time => {
      if (time > maxComputationTime) {
        maxComputationTime = time;
      }
    });
    // Ensure that the order from the nodes array is kept for the datasets
    this.chartState.nodes.forEach(rank => {
      let idleTime = maxComputationTime - this.chartState.progress.get(rank);
      idleTime = idleTime / 1000;
      dataset.push({
        label: 'Worker ' + rank,
        data: [idleTime],
        backgroundColor: this.props.workerContext.getWorkerColor(rank),
        stack: 'idle-time'
      });
    });

    const data = {
      labels: ['Idle time'],
      datasets: dataset
    };
    this.chart.data = data;

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
        state.progress.forEach((value, rank) => {
          if (state.active.get(rank)) {
            state.progress.set(rank, value + interval * 1000);
            update = true;
          }
        });
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

IdleTime.propTypes = {
  wsclient: PropTypes.instanceOf(WebSocketClient),
  workerContext: PropTypes.instanceOf(WorkerContext)
};
