import React, {
  Component
} from 'react';
import {
  Chart
} from 'chart.js';
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
      this.chartState = {};

      this.chartState = {
          numWorkers: 1,
          active: [false],
          // The computation time in microseconds
          progress: [0],
          IDfirst: undefined
      };
  }

  componentDidMount() {

      const ctx = document.getElementById("idleTime");
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
          type: 'bar',
          data: [],
          options: {
              responsive: true,
              maintainAspectRatio: false,
              legend: {
                display: false
              },
              tooltips: {
                  // TODO show the current workercontext active worker on change
                  callbacks: {
                      label: customLabel
                  }
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
              },
              options: {
                scales: {
                  xAxes: [{ stacked: true }],
                  yAxes: [{ stacked: true }]
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
          // Note that it is the first to finish if true
          if(this.chartState.IDfirst === undefined){
            this.chartState.IDfirst = data.workerInfo.rank;
          }
          // assume that regionData is passed here
          const workerID = data.workerInfo.rank;
          // Pay attention here that ranks begin from 1 as long as the host does not send data on his own
          this.chartState.active[workerID - 1] = false;
          // insert correct µs time in node value
          this.chartState.progress[workerID - 1] = data.workerInfo.computationTime;
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
              progress: progress,
              IDfirst: undefined
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
              const activeSegment = this.chart.data.datasets[0]._meta[0].data[activeWorker];
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
      return (<div className="idleTime"><canvas id="idleTime"></canvas></div>);
  }

  updateChart(animationDuration) {
      const progress = this.chartState.progress;
      const dataset = [];
      for (let i = 0; i < progress.length; i++) {
        let idleTime = 0;
        if(this.chartState.IDfirst !== undefined){
          idleTime = progress[this.chartState.IDfirst] - progress[i];
        }
        dataset.push({
          label: "Worker " + i,
          data: [idleTime],
          backgroundColor: this.props.workerContext.getWorkerColor(i),
          stack: 'idle-time'
        });
      }
      
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

IdleTime.propTypes = {
  wsclient: PropTypes.instanceOf(WebSocketClient),
  workerContext: PropTypes.instanceOf(WorkerContext)
};