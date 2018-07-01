import React, { Component } from 'react';
import ReactDOM from 'react-dom';
// Bootstrap
import 'bootstrap/dist/css/bootstrap.css';
// Custom Components
import NodeList from './Node';
import { TileDisplay } from './tileDisplay/TileDisplay';
import registerServiceWorker from './registerServiceWorker';
import './Index.css';

class App extends Component {
  render() {
    return (
      <div>
        <NodeList />
        <TileDisplay />
      </div>
    );
  }
}

ReactDOM.render(<App />, document.getElementById('root'));
registerServiceWorker();
