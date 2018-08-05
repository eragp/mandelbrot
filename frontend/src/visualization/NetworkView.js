import React, { Component } from "react";
import PropTypes from "prop-types";
import WebSocketClient from "../connection/WSClient";

import { DataSet, Network } from "vis";

import "./NetworkView.css";
// TODO include material design icon copyright notice
// TODO find a way to color nodes
import workerImage from "./img/worker.backgroundCircle.svg";
import serverImage from "./img/server.backgroundCircle.svg";
import applicationImage from "./img/application.backgroundCircle.svg";
import WorkerContext from "../misc/WorkerContext";

// TODO highlight node on workercontext active worker change

export default class NetworkView extends Component {
  componentDidMount() {
    const options = {
      autoResize: true,
      physics: {
        enabled: true,
        hierarchicalRepulsion: {
          solver: "forceAtlas2Based",
          centralGravity: 10
        }
      },
      edges: {
        smooth: {
          enabled: false
        },
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
      document.getElementById("nodeNetwork"),
      {
        nodes: new DataSet(),
        edges: new DataSet()
      },
      options
    );

    this.network.on("hoverNode", node => {
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
    this.props.wsclient.registerRegion(data => {
      const nodes = [];
      for (const worker of data.regions) {
        nodes.push(worker.rank);
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

  /**
   * Updates the Dataset of the network according to the received number of workers
   */
  renderNetwork() {
    let nodes = [];

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

    let edges = [];
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
        color: color,
        font: {
          color: color
        },
        level: level
      });
      edges.push({
        from: 1,
        to: i + 2,
        color: {
          color: color,
          hover: color,
          highlight: color
        },
        level: level
      });
    });

    this.network.setData({
      nodes: new DataSet(nodes),
      edges: new DataSet(edges)
    });

    this.network.fit();
  }

  render() {
    return <div id="nodeNetwork" />;
  }
}

NetworkView.propTypes = {
  wsclient: PropTypes.instanceOf(WebSocketClient),
  workerContext: PropTypes.instanceOf(WorkerContext)
};
