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

class App extends Component {
  render() {
    let websocketclient = new WebSocketClient();

    return (
      <div>
        <TileDisplay wsclient={websocketclient}/>
        <NodeProgress wsclient={websocketclient}/>
      </div>
    );
  }
}

ReactDOM.render(<App />, document.getElementById('root'));
registerServiceWorker();
