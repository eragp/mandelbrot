import React, { Component } from 'react';
import PropTypes from 'prop-types';
import './Node.css';

function Node(props) {
  return (
    <li>
      <table>
        <tbody>
          <tr>
            <td className="Node-name">Node-{props.id}</td>
            <td>
              <div className="progress">
                <div
                  className="progress-bar"
                  role="progressbar"
                  style={{ width: props.usage + '%' }}
                  aria-valuenow={props.usage}
                  aria-valuemin="0"
                  aria-valuemax="100"
                />
              </div>
            </td>
            <td>
              Tile: ({props.tileX}, {props.tileY})
            </td>
          </tr>
        </tbody>
      </table>
    </li>
  );
}

Node.propTypes = {
  id: PropTypes.number.isRequired,
  usage: PropTypes.number.isRequired
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
    for (let i = 0; i < 10; i++) {
      nodes.push(
        <Node
          key={i}
          id={i}
          usage={randomNumber(50, 101)}
          tileX={randomNumber(0, 20)}
          tileY={randomNumber(0, 20)}
        />
      );
    }
    return nodes;
  }

  // rerender the list of nodes every 2 seconds
  componentDidMount() {
    this.interval = setInterval(
      () => this.setState({ nodes: this.getNodes() }),
      1000
    );
  }

  render() {
    return (
      <div>
        <h2>Workers</h2>
        <ul className="list-unstyled">{this.state.nodes}</ul>
      </div>
    );
  }
}

const randomNumber = (min, max) => {
  return Math.floor(Math.random() * max) + min;
};

export default NodeList;
