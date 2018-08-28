import * as React from "react";
import { Chart, ChartConfiguration } from "chart.js";
import WebSocketClient from "../connection/WSClient";

import "./NodeProgress.css";
import WorkerContext from "../misc/WorkerContext";

export interface NodeProgressProps {
  workerContext: WorkerContext;
  wsClient: WebSocketClient;
}

interface ChartState {
  workers: Worker[];
}
interface Worker {
  rank: number;
  name: string;
  animating: boolean;
  computationTime: number;
}
/**
 * Shows the computation time of invoked workers
 * Additional documentation on the type of used chart: https://www.chartjs.org/docs/latest/
 */
export default class NodeProgress extends React.Component<NodeProgressProps, {}> {
  private websocketClient: WebSocketClient;
  private workerContext: WorkerContext;
  private chartState: ChartState;
  private chart: Chart;

  private hoveredItem: any;
  private redraw: NodeJS.Timer;

  constructor(props: NodeProgressProps) {
    super(props);
    this.websocketClient = props.wsClient;
    this.workerContext = props.workerContext;
    this.chartState = {
      workers: [
        {
          rank: 0,
          name: "Worker 0",
          animating: false,
          computationTime: 1
        }
      ]
    };
  }

  componentDidMount() {
    const canvas = document.getElementById("nodeProgress") as HTMLCanvasElement;
    const ctx = canvas.getContext("2d") as CanvasRenderingContext2D;
    const renderLabel = (tooltipItem: any, data: any) => {
      let label = data.labels[tooltipItem.index];
      if (label) {
        label += ": ";
      } else {
        label = "";
      }
      label += data.datasets[tooltipItem.datasetIndex].data[tooltipItem.index] + " µs";
      return label;
    };

    this.chart = new Chart(ctx, {
      type: "doughnut",
      data: [],
      options: {
        responsive: true,
        maintainAspectRatio: false,
        tooltips: {
          callbacks: {
            label: renderLabel
          }
        },
        title: {
          display: true,
          position: "bottom",
          text: ["Total node computation time:", "0 µs"]
        },
        onHover: event => {
          // change workerContext active worker on hover
          const data = this.chart.getElementsAtEvent(event)[0];
          if (data) {
            this.workerContext.setActiveWorker(this.chartState.workers[data._index].rank);
            this.hoveredItem = data;
          } else if (this.hoveredItem) {
            this.workerContext.setActiveWorker(undefined);
            this.hoveredItem = undefined;
          }
        }
      }
    } as ChartConfiguration);

    this.updateChart();
    this.startProgressAnimation();

    // register workers at websocket client
    // so that they are set inactive when the first tile/region
    // by them comes in
    this.websocketClient.registerRegionData(data => {
      // Stop corresponding worker progress bar
      // assume that regionData is passed here
      const workerID = data.workerInfo.rank as number;
      var worker = this.chartState.workers.find(w => w.rank === workerID);
      if (worker === undefined || !worker.animating) {
        return;
      }
      // Pay attention here that ranks begin from 1 as long as the host does not send data on his own
      worker.animating = false;
      // insert correct µs time in node value
      worker.computationTime = data.workerInfo.computationTime;
      this.updateChart(1000);
      console.log(`Stopped Worker ${workerID}`);
    });

    this.websocketClient.registerRegion(data => {
      // Stop redrawing
      this.stopProgressAnimation();
      // Reset node progress
      var workers: Worker[] = [];

      const animationDuration = 750;
      for (let region of data.regions) {
        workers.push({
          rank: region.nodeID,
          name: "Worker " + region.nodeID,
          animating: true,
          computationTime: animationDuration * 1000
        });
      }
      this.chartState = {
        workers: workers
      };
      this.updateChart(animationDuration);
      // Start redrawing as soon as animation has finished
      setTimeout(() => {
        this.startProgressAnimation();
      }, animationDuration);
    });

    // Highlight segment on active worker change
    // Inspired by https://github.com/chartjs/Chart.js/issues/1768
    this.props.workerContext.subscribe(activeWorker => {
      // Activate new tooltip if necessary
      if (activeWorker !== undefined) {
        const workerIndex = this.chartState.workers.findIndex(w => w.rank === activeWorker);
        const activeSegment = this.chart.data.datasets[0]._meta[1].data[workerIndex];
        this.chart.tooltip.initialize();
        this.chart.tooltip._active = [activeSegment];
        this.chart.data.datasets[0]._meta[1].controller.setHoverStyle(activeSegment);
        this.hoveredItem= activeSegment;
      } else {
        // Remove tooltip
        this.chart.data.datasets[0]._meta[1].controller.removeHoverStyle(this.hoveredItem);
        this.chart.tooltip._active = [];
      }
      // Update chart
      this.chart.tooltip.update(true);
      this.chart.render(this.chart.options.hover.animationDuration, false);
    });
  }

  render() {
    return (
      <div className="nodeProgress">
        <canvas id="nodeProgress" />
      </div>
    );
  }

  private updateChart(animationDuration: number = 0) {
    var labels: string[] = [];
    var values: number[] = [];
    var colorSet: string[] = [];
    // => Label/ value index is the index of the rank in the node array
    this.chartState.workers.forEach(worker => {
      labels.push(worker.name);
      colorSet.push(this.workerContext.getWorkerColor(worker.rank));
      values.push(worker.computationTime);
    });

    const data = {
      labels: labels,
      datasets: [
        {
          data: values,
          backgroundColor: colorSet
        }
      ]
    };
    this.chart.data = data;

    let computationTime = 0;
    this.chartState.workers.forEach(worker => {
      computationTime += worker.computationTime;
    });
    this.chart.options.title.text[1] = computationTime + " µs";
    this.chart.update(animationDuration);
  }

  /**
   * Start redrawing the current node computation time every 50 milliseconds
   */
  private startProgressAnimation() {
    // Interval in milliseconds
    const intervalRate = 50;
    this.redraw = setInterval(
      state => {
        let updated = false;
        state.workers.forEach((worker: any) => {
          if (worker.animating) {
            worker.computationTime += intervalRate * 1000;
            updated = true;
          }
        });
        if (updated) {
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
  private stopProgressAnimation() {
    clearInterval(this.redraw);
  }
}
