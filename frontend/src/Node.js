import React, { Component } from 'react';
import PropTypes from 'prop-types';
import './Node.css';
import { Progress } from 'reactstrap'

function Node(props) {
  return (
    <li>
      <div className="workerDiv">
        <Progress animated={props.animated} value={props.usage} className="workerTime"> Worker {props.id} - {props.time} µs </Progress>
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
      // The computation time in microseconds
      progress: new Array(this.numWorkers)
    };
  }

  getNodes() {
    let nodes = [];
    for (let i = 0; i < this.state.numWorkers; i++) {
      // TODO use a nice function for plotting milliseconds on progress
      let n = this.state.progress[i] / 100000;
      let active = this.state.active[i];
      let time = this.state.progress[i];
      nodes.push(
        <Node
          key={i}
          id={i}
          usage={n}
          animated={active}
          time={time}
        />
      );
    }
    return nodes;
  }

  // rerender the list of nodes every 2 seconds
  componentDidMount() {
    // Interval in milliseconds
    let interval = 250;
    this.interval = setInterval(
      () => {      
        this.setState((oldState) => {
          for(let i = 0; i < oldState.numWorkers; i++){
            if(oldState.active[i]){
              oldState.progress[i] += interval * 1000;
            }
          }
          return oldState;
        });
      },
      interval
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
