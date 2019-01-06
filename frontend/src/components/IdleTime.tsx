import * as React from "react";
import {
  Chart,
  ChartTooltipItem,
  ChartData,
  ChartConfiguration,
  ChartDataSets,
  ChartOptions,
  ChartHoverOptions
} from "chart.js";
import WebSocketClient from "../connection/WSClient";
import { GroupObservable } from "../misc/Observable";

import "./IdleTime.css";
import { RegionGroup, groupRegions } from "../connection/RegionGroup";
import { WorkerInfo } from "../connection/ExchangeTypes";
import { usToString } from "../misc/Conversion";

interface IdleTimeProps {
  group: GroupObservable;
  wsclient: WebSocketClient;
}

interface IdleTimeState {
  nodes: RegionGroup[];
  active: Map<number, boolean>;
  progress: Map<number, number>;
}

// Display idle time in seconds
const factor = 1e6;
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

    const pseudoWorker: WorkerInfo = {
      rank: 0,
      computationTime: 0,
      mpiTime: 0,
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
        fractal: "mandelbrot",
        regionCount: 0
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
    // TODO does not work
    const customLabel = (tooltipItem: any, data: any) =>
      // needs to be multiplied by factor, because the cart data is in seconds
      usToString(data.datasets[tooltipItem.datasetIndex].data[tooltipItem.index] * factor);

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
            this.props.group.set(
              // @ts-ignore: data does not have complete .d.ts file
              this.chartState.nodes[data._datasetIndex].id
            );
            this.hoveredItem = data;
          } else if (this.hoveredItem) {
            this.props.group.set(undefined);
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
                suggestedMax: 10
              },
              scaleLabel: {
                display: true,
                labelString: "s"
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
      this.stopNodeProgress();
      // Stop corresponding worker progress bar
      // assume that regionData is passed here

      // Note that it is not active anymore
      // especially this one worker (part of the whole group)
      this.chartState.active.set(data.workerInfo.rank, false);
      // insert correct µs time in node value
      // Problem: when setting progress for whole group, we remove the progress
      // up to this point estimated for all other nodes of the group
      // => store progress for each node separated
      this.chartState.progress.set(data.workerInfo.rank, data.workerInfo.computationTime);
      this.updateChart(0);
      this.initNodeProgress();
    });

    this.props.wsclient.registerRegion(groups => {
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
      if (groupIndex) {
        const index = this.chartState.nodes.findIndex(g => g.id === groupIndex);
        // @ts-ignore: does not have complete .d.ts file
        const activeSegment = (this.chart.data.datasets as ChartDataSets[])[index]._meta[0].data[0];
        // @ts-ignore: does not have complete .d.ts file
        this.chart.tooltip.initialize();
        // @ts-ignore: does not have complete .d.ts file
        this.chart.tooltip._active = [activeSegment];

        (this.chart.data.datasets as ChartDataSets[])[
          index
          // @ts-ignore: does not have complete .d.ts file
        ]._meta[0].controller.setHoverStyle(activeSegment);
        this.hoveredSegment = activeSegment;
      } else {
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
    const groupCompTime = (group: RegionGroup) => {
      let compTime = 0;
      for (const region of group.getLeafs()) {
        compTime += this.chartState.progress.get(region.id) as number;
      }
      return compTime;
    };
    let maxComputationTime = 0;
    for (const g of this.chartState.nodes) {
      const compTime = groupCompTime(g);
      if (compTime > maxComputationTime) {
        maxComputationTime = compTime;
      }
    }
    // Ensure that the order from the nodes array is kept for the datasets
    this.chartState.nodes.forEach(group => {
      const rank = group.id;
      const groupSize = group.getLeafs().length;
      // IdleTime is displayed in seconds, averaged over the size of the group
      const idleTime = (maxComputationTime - groupCompTime(group)) / (groupSize * factor);
      datasets.push({
        label: "Group " + rank,
        data: [idleTime],
        backgroundColor: this.props.group.getColor(rank),
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
        this.chartState.nodes.forEach(group => {
          for (const region of group.getLeafs()) {
            if (state.active.get(region.id)) {
              state.progress.set(
                region.id,
                (state.progress.get(region.id) as number) + interval * 1000
              );
              update = true;
            }
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
