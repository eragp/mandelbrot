import * as React from "react";
import {
  Chart,
  ChartTooltipItem,
  ChartData,
  ChartConfiguration,
  ChartDataSets,
  ChartOptions,
  ChartHoverOptions,
} from "chart.js";
import WebSocketClient from "../connection/WSClient";

import "./IdleTime.css";
import WorkerContext from "../misc/WorkerContext";

interface IdleTimeProps {
  wsclient: WebSocketClient;
  workerContext: WorkerContext;
}

interface IdleTimeState {
  nodes: number[];
  active: Map<number, boolean>;
  progress: Map<number, number>;
}
/**
 * Shows the computation time of invoked workers
 * Additional documentation on the type of used chart: https://www.chartjs.org/docs/latest/
 */
export default class IdleTime extends React.Component<IdleTimeProps, {}> {
  private chartState: IdleTimeState;
  private chart: Chart;
  private interval: NodeJS.Timer;

  private hoveredItem: any;
  private hoveredSegment: any;

  constructor(props: IdleTimeProps) {
    super(props);

    this.chartState = {
      nodes: [0],
      active: new Map([[0, false]]),
      // The computation time in microseconds
      progress: new Map([[0, 1]])
    };
  }

  public componentDidMount() {
    const customLabel = (tooltipItem: ChartTooltipItem, data: ChartData) => {
      if (!data.datasets || !tooltipItem.datasetIndex || !tooltipItem.index) {
        return "";
      }
      const dataset = data.datasets[tooltipItem.datasetIndex];
      if (!dataset.data) {
        return "";
      }
      let label = dataset.label;

      if (label) {
        label += ": ";
      } else {
        label = "";
      }
      label += dataset.data[tooltipItem.index];
      label += " ms";
      return label;
    };

    const ctx = document.getElementById("idleTime") as HTMLCanvasElement;
    const config: ChartConfiguration = {
      type: "bar",
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
          const data = this.chart.getDatasetAtEvent(event)[0] as ChartDataSets;
          if (data) {
            this.props.workerContext.setActiveWorker(
              // @ts-ignore: data does not have complete .d.ts file
              this.chartState.nodes[data._datasetIndex]
            );
            this.hoveredItem = data;
          } else if (this.hoveredItem) {
            this.props.workerContext.setActiveWorker(undefined);
            this.hoveredItem = undefined;
          }
        },
        scales: {
          yAxes: [
            {
              type: "linear",
              stacked: true,
              ticks: {
                beginAtZero: true,
                min: 0,
                // @ts-ignore: suggestedMax is not in TickOptions.d.ts
                suggestedMax: 10000
              },
              scaleLabel: {
                display: true,
                labelString: "ms"
              }
            }
          ]
        }
      }
    };
    this.chart = new Chart(ctx, config);
    this.updateChart();

    this.initNodeProgress();

    // register workers at websocket client
    // so that they are set inactive when the first tile/region
    // by them comes in
    this.props.wsclient.registerRegionData(data => {
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

    this.props.wsclient.registerRegion(group => {
      // Stop redrawing
      this.stopNodeProgress();
      // Reset node progress
      const nodes = [];
      const active = new Map();
      const progress = new Map();

      let animationDuration = 750;

      for (let region of group) {
        nodes.push(region.id);
        active.set(region.id, true);
        progress.set(region.id, animationDuration * 1000);
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
        // @ts-ignore: does not have complete .d.ts file
        const activeSegment = (this.chart.data.datasets as ChartDataSets[])[workerIndex]._meta[0]
          .data[0];
        // @ts-ignore: does not have complete .d.ts file
        this.chart.tooltip.initialize();
        // @ts-ignore: does not have complete .d.ts file
        this.chart.tooltip._active = [activeSegment];
        (this.chart.data.datasets as ChartDataSets[])[
          workerIndex
          // @ts-ignore: does not have complete .d.ts file
        ]._meta[0].controller.setHoverStyle(activeSegment);
        this.hoveredSegment = activeSegment;
      } else {
        // Remove tooltip
        // @ts-ignore: does not have complete .d.ts file
        (this.chart.data.datasets as ChartDataSets[])[0]._meta[0].controller.removeHoverStyle(
          this.hoveredSegment
        );
        // @ts-ignore: does not have complete .d.ts file
        this.chart.tooltip._active = [];
      }
      // Update chart
      // @ts-ignore: does not have complete .d.ts file
      this.chart.tooltip.update(true);
      this.chart.render(
        ((this.chart.config.options as ChartOptions).hover as ChartHoverOptions).animationDuration,
        false
      );
    });
  }

  render() {
    return (
      <div className="idleTime">
        <canvas id="idleTime" />
      </div>
    );
  }

  private updateChart(animationDuration?: number) {
    const datasets: ChartDataSets[] = [];
    let maxComputationTime = 0;
    this.chartState.progress.forEach(time => {
      if (time > maxComputationTime) {
        maxComputationTime = time;
      }
    });
    // Ensure that the order from the nodes array is kept for the datasets
    this.chartState.nodes.forEach(rank => {
      let idleTime = maxComputationTime - (this.chartState.progress.get(rank) as number);
      idleTime = idleTime / 1000;
      datasets.push({
        label: "Worker " + rank,
        data: [idleTime],
        backgroundColor: this.props.workerContext.getWorkerColor(rank),
        stack: "idle-time"
      });
    });

    const data = {
      labels: ["Idle time"],
      datasets
    };
    this.chart.data = data;

    this.chart.update(animationDuration);
  }

  /**
   * Start redrawing the current node computation time every 50 milliseconds
   */
  private initNodeProgress() {
    // Interval in milliseconds
    const interval = 50;
    this.interval = setInterval(
      (state: IdleTimeState) => {
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
  private stopNodeProgress() {
    clearInterval(this.interval);
  }
}
