import * as React from "react";
import WebSocketClient from "../connection/WSClient";

import { DataSet, Network, Node, Edge, Options as NetworkOptions} from "vis";

import "./NetworkView.css";
// TODO include material design icon copyright notice
// TODO find a way to color nodes
import workerImage from "./img/worker.backgroundCircle.svg";
import serverImage from "./img/server.backgroundCircle.svg";
import applicationImage from "./img/application.backgroundCircle.svg";
import WorkerContext from "../misc/WorkerContext";

interface NetworkViewProps {
  workerContext: WorkerContext;
  wsclient: WebSocketClient;
}

interface NetworkHoverEvent {
  pointer: any;
  event: MouseEvent;
  node: number;
}
// TODO highlight node on workercontext active worker change

export default class NetworkView extends React.Component<NetworkViewProps, {}> {

  private network: Network;
  private networkState: {
    nodes: number[];
  }

  public componentDidMount() {
    const options: NetworkOptions = {
      autoResize: true,
      physics: {
        enabled: true,
        solver: "forceAtlas2Based",
        hierarchicalRepulsion: {
          centralGravity: 10,
        },
      },
      edges: {
        smooth: false,
        color: {
          color: "black",
        },
      },
      layout: {
        hierarchical: {
          enabled: true,
          direction: "LR",
          sortMethod: "directed",
        }
      },
      interaction: {
        dragNodes: false,
        dragView: false,
        hover: true,
        zoomView: false,
        selectable: false,
      }
    };

    this.network = new Network(
      document.getElementById("nodeNetwork") as HTMLCanvasElement,
      {
        nodes: new DataSet(),
        edges: new DataSet()
      },
      options,
    );

    this.network.on("hoverNode", (node: NetworkHoverEvent) => {
      if (node.node >= 2) {
        this.props.workerContext.setActiveWorker(this.networkState.nodes[node.node - 2]);
      }
    });
    this.network.on("blurNode", node => {
      if (node.node >= 2) {
        this.props.workerContext.setActiveWorker(undefined);
      }
    });

    this.networkState = {
      nodes: [0]
    };
    this.renderNetwork();

    /**
     * Redraw graph when information about backend becomes available
     */
    this.props.wsclient.registerRegion(regions => {
      const nodes = [];
      for (const region of regions) {
        nodes.push(region.rank);
      }
      if (this.networkState.nodes !== nodes) {
        this.networkState.nodes = nodes;
        this.renderNetwork();
      }
    });

    /**
     * Register for changing active worker
     */
    this.props.workerContext.subscribe(activeNode => {
      if (activeNode !== undefined) {
        this.network.selectNodes([this.networkState.nodes.indexOf(activeNode) + 2]);
      } else {
        this.network.unselectAll();
      }
    });
  }

  public render() {
    return <div id="nodeNetwork" />;
  }

  /**
   * Updates the Dataset of the network according to the received number of workers
   */
  private renderNetwork() {
    const nodes: Node[] = [];

    nodes.push({
      id: 0,
      label: "Frontend",
      image: applicationImage,
      shape: "image",
      level: 0,
      fixed: true,
    });
    nodes.push({
      id: 1,
      label: `Backend-Host`,
      image: serverImage,
      shape: "image",
      level: 1,
      fixed: true
    });

    const edges: Edge[] = [];
    edges.push({
      from: 0,
      to: 1
    });
    this.networkState.nodes.forEach((rank, i) => {
      const color = this.props.workerContext.getWorkerColor(rank);
      const level = Math.floor(i / 2) + 2;
      nodes.push({
        id: i + 2,
        label: `Worker ${rank}`,
        image: workerImage,
        shape: "image",
        font: {
          color: color,
        },
        level,
      });
      edges.push({
        from: 1,
        to: i + 2,
        color: {
          color: color,
          hover: color,
          highlight: color
        },
      });
    });

    this.network.setData({
      nodes: new DataSet(nodes),
      edges: new DataSet(edges)
    });

    this.network.fit();
  }
}
