import React, { Component } from 'react';
import ReactDOM from 'react-dom';
// Bootstrap
import 'bootstrap/dist/css/bootstrap.css';
// Custom Components
import TileDisplay from './tileDisplay/TileDisplay';
import registerServiceWorker from './registerServiceWorker';
import NodeProgress from './visualization/NodeProgress';
import WebSocketClient from './connection/WSClient';
import BalancerChoice from './visualization/BalancerChoice';
import BalancerPolicy from './misc/BalancerPolicy';

// CSS
import './Index.css';

class App extends Component {
  render() {
    let websocketclient = new WebSocketClient();
    let balancerPolicy = new BalancerPolicy();

    return (
      <div>
        <div class="mainTop">
          <TileDisplay wsclient={websocketclient} balancerPolicy={balancerPolicy}/>
        </div> <div class="mainBottom row">
          <div class="col-3">
            <BalancerChoice balancerPolicy={balancerPolicy}/>
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
