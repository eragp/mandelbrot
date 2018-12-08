import * as React from "react";
import {
  Chart,
  ChartConfiguration,
  ChartDataSets,
  ChartOptions,
  ChartTitleOptions,
  ChartHoverOptions
} from "chart.js";
import WebSocketClient from "../connection/WSClient";

import "./NodeProgress.css";
import WorkerContext from "../misc/WorkerContext";

interface NodeProgressProps {
  workerContext: WorkerContext;
  wsClient: WebSocketClient;
}
interface ChartState {
  nodes: number[];
  active: Map<number, boolean>;
  progress: Map<number, number>;
}
/**
 * Shows the computation time of invoked workers
 * Additional documentation on the type of used chart: https://www.chartjs.org/docs/latest/
 */
export default class NodeProgress extends React.Component<NodeProgressProps, {}> {
  private websocketClient: WebSocketClient;
  private chartState: ChartState;
  private chart: Chart;

  private hoveredItem: any;
  private hoveredSegment: any;
  private interval: NodeJS.Timer;

  constructor(props: NodeProgressProps) {
    super(props);
    this.websocketClient = props.wsClient;
    this.chartState = {
      nodes: [0],
      active: new Map([[0, false]]),
      // The computation time in microseconds
      progress: new Map([[0, 1]])
    };
  }

  public componentDidMount() {
    const ctx = document.getElementById("nodeProgress") as HTMLCanvasElement;
    const customLabel = (tooltipItem: any, data: any) => {
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
            label: customLabel
          }
        },
        title: {
          display: true,
          position: "bottom",
          text: ["Total node computation time:", "0 µs"]
        },
        onHover: event => {
          // change workercontext active worker on hover
          const data = this.chart.getElementsAtEvent(event)[0] as ChartDataSets;
          if (data) {
            // @ts-ignore: does not have complete .d.ts file
            this.props.workerContext.setActiveWorker(this.chartState.nodes[data._index]);
            this.hoveredItem = data;
          } else if (this.hoveredItem) {
            this.props.workerContext.setActiveWorker(undefined);
            this.hoveredItem = undefined;
          }
        }
      }
    } as ChartConfiguration);
    this.updateChart();

    this.initNodeProgress();

    // register workers at websocket client
    // so that they are set inactive when the first tile/region
    // by them comes in
    this.websocketClient.registerRegionData(data => {
      // Stop corresponding worker progress bar
      // assume that regionData is passed here
      const workerID = data.workerInfo.rank;
      // Pay attention here that ranks begin from 1 as long as the host does not send data on his own
      this.chartState.active.set(workerID, false);
      // insert correct µs time in node value
      this.chartState.progress.set(workerID, data.workerInfo.computationTime);
      this.updateChart();
    });

    this.websocketClient.registerRegion(group => {
      // Stop redrawing
      this.stopNodeProgress();
      // Reset node progress
      const nodes = [];
      const active = new Map();
      const progress = new Map();

      const animationDuration = 750;
      for (const region of group) {
        nodes.push(region.rank);
        active.set(region.rank, true);
        progress.set(region.rank, animationDuration * 1000);
      }
      this.chartState = {
        nodes,
        active,
        progress
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
      const datasets = this.chart.data.datasets as ChartDataSets[];
      if (datasets === undefined) {
        return;
      }
      const tooltip = (this.chart as any).tooltip;
      if (activeWorker !== undefined) {
        const workerIndex = this.chartState.nodes.indexOf(activeWorker);
      // @ts-ignore: does not have complete .d.ts file
        const activeSegment = datasets[0]._meta[1].data[workerIndex];
        tooltip.initialize();
        tooltip._active = [activeSegment];
      // @ts-ignore: does not have complete .d.ts file
        datasets[0]._meta[1].controller.setHoverStyle(activeSegment);
        this.hoveredSegment = activeSegment;
      } else {
        // Remove tooltip
      // @ts-ignore: does not have complete .d.ts file
        datasets[0]._meta[1].controller.removeHoverStyle(this.hoveredSegment);
        tooltip._active = [];
      }
      // Update chart
      tooltip.update(true);
      const animationDuration = ((this.chart.config.options as ChartOptions)
        .hover as ChartHoverOptions).animationDuration;
      this.chart.render(animationDuration, false);
    });
  }

  public render() {
    return (
      <div className="nodeProgress">
        <canvas id="nodeProgress" />
      </div>
    );
  }

  private updateChart(animationDuration: number = 0) {
    const labels: string[] = [];
    const values: number[] = [];
    const colorSet: string[] = [];
    // => Label/ value index is the index of the rank in the node array
    this.chartState.nodes.forEach(rank => {
      labels.push("Worker " + rank);
      colorSet.push(this.props.workerContext.getWorkerColor(rank));
      values.push(this.chartState.progress.get(rank) as number);
    });

    const data = {
      labels,
      datasets: [
        {
          data: values,
          backgroundColor: colorSet
        }
      ]
    };
    this.chart.data = data;

    let computationTime = 0;
    this.chartState.progress.forEach(value => {
      computationTime += value;
    });
    (((this.chart.config.options as ChartOptions).title as ChartTitleOptions).text as string[])[1] =
      computationTime + " µs";

    this.chart.update(animationDuration);
  }

  /**
   * Start redrawing the current node computation time every 50 milliseconds
   */
  private initNodeProgress() {
    // Interval in milliseconds
    const intervalRate = 50;
    this.interval = setInterval(
      (state: ChartState) => {
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
  private stopNodeProgress() {
    clearInterval(this.interval);
  }
}
