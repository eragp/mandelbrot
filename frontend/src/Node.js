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
      numWorkers: 0,
      active: new Array(this.numWorkers),
      progress: new Array(this.numWorkers)
    };
  }

  getNodes() {
    let nodes = [];
    for (let i = 0; i < this.state.numWorkers; i++) {
      let n = this.state.progress[i];
      let active = this.state.active[i];
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
      () => {      
        this.setState((oldState) => {
          for(let i = 0; i < oldState.numWorkers; i++){
            if(oldState.active[i]){
              oldState.progress[i] += 5;
            }
          }
          return oldState;
        });
      },
      500
    );
  }

  render() {
    let nodes = this.getNodes()
    return (
      <div>
        <ul className="list-unstyled">{nodes}</ul>
      </div>
    );
  }
}

const randomNumber = (min, max) => {
  return Math.floor(Math.random() * max) + min;
};

export default NodeList;
