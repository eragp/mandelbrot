import React, { Component } from 'react';
import ReactDOM from 'react-dom';
// Bootstrap
import 'bootstrap/dist/css/bootstrap.css';
// Custom Components
import TileDisplay from './tileDisplay/TileDisplay';
import registerServiceWorker from './registerServiceWorker';
import './Index.css';
import NodeProgress from './visualization/NodeProgress';
import WebSocketClient from './connection/WSClient';
import BalancerChoice from './visualization/BalancerChoice';

class App extends Component {
  render() {
    let websocketclient = new WebSocketClient();

    return (
      <div>
        <div class="mainTop">
          <TileDisplay wsclient={websocketclient}/>
        </div> <div class="mainBottom row">
          <div class="col-3">
            <BalancerChoice />
          </div>
          <div class="col-6">
            <span />
          </div>
          <div class="col-3">
            <NodeProgress wsclient={websocketclient}/>
          </div>
        </div>
      </div>
    );
  }
}

ReactDOM.render(<App />, document.getElementById('root'));
registerServiceWorker();
