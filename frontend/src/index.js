import React, { Component } from 'react';
import ReactDOM from 'react-dom';
import NodeList from './Node';
import Viewer from './Viewer';
import registerServiceWorker from './registerServiceWorker';

class App extends Component {
  render() {
    return (
      <div>
        <NodeList />
        <Viewer />
      </div>
    );
  }
}

ReactDOM.render(<App />, document.getElementById('root'));
registerServiceWorker();
