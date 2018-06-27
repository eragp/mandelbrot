import React, { Component } from 'react';
import PropTypes from 'prop-types';
import './Node.css';
import { Progress } from 'reactstrap'

function Node(props) {
  return (
    <li>
      <div className="workerDiv">
        <Progress animated={props.animated} value={props.usage} className="workerTime"> Worker {props.id} </Progress>
      </div>
    </li>
  );
}

Node.propTypes = {
  id: PropTypes.number.isRequired,
  usage: PropTypes.number.isRequired,
  striped: PropTypes.string.isRequired
};

class NodeList extends Component {
  constructor(props) {
    super(props);
    this.state = {
      nodes: this.getNodes()
    };
  }

  getNodes() {
    let nodes = [];
    for (let i = 0; i < 3; i++) {
      let n = randomNumber(50, 100);
      let active = n >= 90 ? false : true;
      nodes.push(
        <Node
          key={i}
          id={i}
          usage={n}
          animated={active}
        />
      );
    }
    return nodes;
  }

  // rerender the list of nodes every 2 seconds
  componentDidMount() {
    this.interval = setInterval(
      () => this.setState({ nodes: this.getNodes() }),
      5000
    );
  }

  render() {
    return (
      <div>
        <ul className="list-unstyled">{this.state.nodes}</ul>
      </div>
    );
  }
}

const randomNumber = (min, max) => {
  return Math.floor(Math.random() * max) + min;
};

export default NodeList;
