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
          IDlastActive: 0
      };
  }

  componentDidMount() {

      const ctx = document.getElementById("idleTime");
      const customLabel = (tooltipItem, data) => {
          let label = data.datasets[tooltipItem.datasetIndex].label;

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
                  const data = this.chart.getDatasetAtEvent(event)[0];
                  if(data){
                      this.props.workerContext.setActiveWorker(data._datasetIndex);
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
                  yAxes: [{ 
                    type: 'logarithmic',
                    id: 'idle-time-axis',
                    stacked: true,
                    ticks: {
                      min: 0,
                      max: 10000000
                    }
                  }]
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
          // Pay attention here that ranks begin from 1 as long as the host does not send data on his own
          const workerID = data.workerInfo.rank -1;

          // Note that it is not active anymore
          this.chartState.active[workerID] = false;
          if(this.chartState.IDlastActive === workerID){
            let anotherActive;
            // Find someone else who is active, if none is => stay the same
            this.chartState.active.forEach((active, rank) => {
              if(active){
                anotherActive = rank;
              }
            })
            if(anotherActive !== undefined){
              this.chartState.IDlastActive = anotherActive;
            }
          }
          // insert correct µs time in node value
          this.chartState.progress[workerID] = data.workerInfo.computationTime;
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
              IDlastActive: 0
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
              const activeSegment = this.chart.data.datasets[activeWorker]._meta[0].data[0];
              this.chart.tooltip.initialize();
              this.chart.tooltip._active = [activeSegment];
              this.chart.data.datasets[activeWorker]._meta[0].controller.setHoverStyle(activeSegment);
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
        let idleTime = progress[this.chartState.IDlastActive] - progress[i];
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