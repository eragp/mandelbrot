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

import "./ComputationTime.css";
import {GroupObservable} from "../misc/Observable";
import { RegionGroup, groupRegions} from "../misc/RegionGroup";
import { WorkerInfo } from "../connection/ExchangeTypes";

interface NodeProgressProps {
  group: GroupObservable;
  wsClient: WebSocketClient;
}
interface ChartState {
  nodes: RegionGroup[];
  active: Map<number, boolean>;
  progress: Map<number, number>;
}
/**
 * Shows the computation time of invoked workers
 * Additional documentation on the type of used chart: https://www.chartjs.org/docs/latest/
 */
export default class ComputationTime extends React.Component<NodeProgressProps, {}> {
  private websocketClient: WebSocketClient;
  private chartState: ChartState;
  private chart: Chart;

  private hoveredItem: any;
  private hoveredSegment: any;
  private interval: NodeJS.Timer;

  constructor(props: NodeProgressProps) {
    super(props);
    this.websocketClient = props.wsClient;
    const pseudoWorker: WorkerInfo = {
        rank: 0,
        computationTime: 0,
        region: {
            guaranteedDivisor: 0,
            hOffset: 0,
            vOffset: 0,
            height: 0,
            maxImag: 0,
            maxIteration: 0,
            maxReal: 0,
            minImag: 0,
            minReal: 0,
            validation: 0,
            width: 0,
            fractal: "mandelbrot"
        }
    };
    this.chartState = {
      nodes: groupRegions([pseudoWorker]),
      active: new Map([[0, false]]),
      // The computation time in microseconds
      progress: new Map([[0, 1]])
    };
  }

  public componentDidMount() {
    const ctx = document.getElementById("nodeProgress") as HTMLCanvasElement;
    const customLabel = (tooltipItem: any, data: any) =>
      `${data.labels[tooltipItem.index]}: ${this.usToString(
        data.datasets[tooltipItem.datasetIndex].data[tooltipItem.index] * 10
      )}`;

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
            this.props.group.set(this.chartState.nodes[data._index].id);
            this.hoveredItem = data;
          } else if (this.hoveredItem) {
            this.props.group.set(undefined);
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
      this.stopNodeProgress();
      // Stop corresponding group progress bar
      // assume that regionData is passed here
      // Pay attention here that ranks begin from 1 as long as the host does not send data on his own
      // Do set exactly this region (part of the group) inactive
      this.chartState.active.set(data.workerInfo.rank, false);
      // insert correct µs time in node value
      this.chartState.progress.set(data.workerInfo.rank, data.workerInfo.computationTime);
      this.updateChart(0);
      this.initNodeProgress();
    });

    this.websocketClient.registerRegion(groups => {
      // Stop redrawing
      this.stopNodeProgress();
      // Reset node progress
      const nodes = groups;
      const active = new Map();
      const progress = new Map();

      const animationDuration = 750;
      for (const group of groups) {
        for (const region of group.getLeafs()) {
            active.set(region.id, true);
            progress.set(region.id, animationDuration * 1000);
        }
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
    this.props.group.subscribe(groupIndex => {
      // Activate new tooltip if necessary
      const datasets = this.chart.data.datasets as ChartDataSets[];
      if (datasets === undefined) {
        return;
      }
      const tooltip = (this.chart as any).tooltip;
      if (groupIndex !== undefined) {
        const workerIndex = this.chartState.nodes.findIndex(g => g.id === groupIndex);
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

  public usToString(time: number) {
      const units = ["μs", "ms", "s"];
      let i = 0;
      while (i != units.length && time > 1000) {
        time = time / 1000;
        i++;
      }
      return (Math.round(time * 100) / 100).toFixed(2) + " " + units[i];
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
    const groupCompTime = (group: RegionGroup) => {
        let compTime = 0;
        for (const region of group.getLeafs()) {
            compTime += this.chartState.progress.get(region.id) as number;
        }
        return compTime;
    };
    this.chartState.nodes.forEach(group => {
      const rank = group.id;
      labels.push("Group " + rank);
      colorSet.push(this.props.group.getColor(rank));
      values.push(groupCompTime(group));
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
    (((this.chart.config.options as ChartOptions).title as ChartTitleOptions)
      .text as string[])[1] = this.usToString(computationTime);

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
        this.chartState.nodes.forEach(group => {
          // Check for all regions of the group
          for (const region of group.getLeafs()) {
              if (state.active.get(region.id) === true) {
                state.progress.set(region.id, (state.progress.get(region.id) as number) + intervalRate * 1000);
                update = true;
              }
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
