import * as React from "react";
import WebSocketClient from "../connection/WSClient";

import { DataSet, Network, Node, Edge, Options as NetworkOptions } from "vis";

import "./NetworkView.css";
// TODO include material design icon copyright notice
// TODO find a way to color nodes
import workerImage from "./img/worker.backgroundCircle.svg";
import serverImage from "./img/server.backgroundCircle.svg";
import applicationImage from "./img/application.backgroundCircle.svg";
import { RegionGroup } from "../misc/RegionGroup";
import { GroupObservable } from "../misc/Observable";

interface NetworkViewProps {
  group: GroupObservable;
  wsclient: WebSocketClient;
}

interface NetworkHoverEvent {
  pointer: any;
  event: MouseEvent;
  node: number;
}
// TODO highlight node on workercontext active worker change

const groupToNet = (id: number) => id + 2;
const netToGroup = (id: number) => id - 2;

export default class NetworkView extends React.Component<NetworkViewProps, {}> {
  private network: Network;
  private groups: RegionGroup[];

  public componentDidMount() {
    const options: NetworkOptions = {
      autoResize: true,
      physics: {
        enabled: true,
        solver: "forceAtlas2Based",
        hierarchicalRepulsion: {
          centralGravity: 10
        }
      },
      edges: {
        smooth: false,
        color: {
          color: "black"
        }
      },
      layout: {
        hierarchical: {
          enabled: true,
          direction: "LR",
          sortMethod: "directed"
        }
      },
      interaction: {
        dragNodes: false,
        dragView: false,
        hover: true,
        zoomView: false,
        selectable: false
      }
    };

    this.network = new Network(
      document.getElementById("nodeNetwork") as HTMLCanvasElement,
      {
        nodes: new DataSet(),
        edges: new DataSet()
      },
      options
    );

    this.network.on("hoverNode", (node: NetworkHoverEvent) => {
      if (node.node >= 2) {
        this.props.group.set(node.node - 2);
      }
    });
    this.network.on("blurNode", node => {
      if (node.node >= 2) {
        this.props.group.set(undefined);
      }
    });

    this.groups = [];
    this.renderNetwork();

    /**
     * Redraw graph when information about backend becomes available
     */
    this.props.wsclient.registerRegion(groups => {
      if (this.groups !== groups) {
        this.groups = groups;
        this.renderNetwork();
      }
    });

    /**
     * Register for changing active worker
     */
    // this.props.group.subscribe(activeNode => {
    //   if (activeNode) {
    //     this.network.selectNodes([activeNode + 2]);
    //   } else {
    //     this.network.unselectAll();
    //   }
    // });
  }

  public render() {
    return <div id="nodeNetwork" />;
  }

  /**
   * Updates the Dataset of the network according to the received number of workers
   */
  private renderNetwork() {
    const nodes: Node[] = [];
    const edges: Edge[] = [];

    // set nodes for frontend and Backend host
    nodes.push({
      id: 0,
      label: "Frontend",
      image: applicationImage,
      shape: "image",
      level: 0,
      fixed: true
    });
    nodes.push({
      id: 1,
      label: `Backend-Host`,
      image: serverImage,
      shape: "image",
      level: 1,
      fixed: true
    });
    edges.push({
      from: 0,
      to: 1
    });

    this.groups.forEach((group, i) => {
      const color = this.props.group.getColor(group.id);
      const level = Math.floor(i / 2) + 2;
      const id = group.id + 2;
      // push group node
      nodes.push({
        id: id,
        label: group.isGroup() ? `Group ${group.id}` : `Worker ${group.id}`,
        image: workerImage,
        shape: "image",
        font: {
          color: color
        },
        level
      });
      edges.push({
        from: 1,
        to: id,
        color: {
          color: color,
          hover: color,
          highlight: color
        }
      });
      // if (i == 0) {
      //   // push nodes
      //   group.getRanks().forEach((r, i) => {
      //     nodes.push({
      //       id: group.id * 100 + r,
      //       label: `Worker ${r}`,
      //       image: workerImage,
      //       shape: "image",
      //       font: {
      //         color: color
      //       },
      //       level: level
      //     });
      //     edges.push({
      //       from: group.id + 2,
      //       to: group.id * 100 + r,
      //       color: {
      //         color: color,
      //         hover: color,
      //         highlight: color
      //       }
      //     });
      //   });
      // }
    });

    this.network.setData({
      nodes: new DataSet(nodes),
      edges: new DataSet(edges)
    });

    this.network.fit();
  }
}
