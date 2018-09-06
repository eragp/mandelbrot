import * as React from "react";
import * as ReactDOM from "react-dom";
// Bootstrap
import "bootstrap/dist/css/bootstrap.css";
// Custom Components
import TileDisplay from "./tileDisplay/TileDisplay.jsx";
import registerServiceWorker from "./registerServiceWorker";
import NodeProgress from "./visualization/NodeProgress";
import WebSocketClient from "./connection/WSClient";
import BalancerChoice from "./visualization/BalancerChoice.jsx";
import BalancerPolicy from "./misc/BalancerPolicy";
import WorkerContext from "./misc/WorkerContext";
import NetworkView from "./visualization/NetworkView.jsx";
import IdleTime from "./visualization/IdleTime.jsx";

// CSS
import "./index.css";

class App extends React.Component<{}, {}> {
  render() {
    const websocketclient = new WebSocketClient();
    const balancerPolicy = new BalancerPolicy();
    const workerContext = new WorkerContext();

    return (
      <div>
        <div className="mainTop">
          <TileDisplay
            workerContext={workerContext}
            wsclient={websocketclient}
            balancerPolicy={balancerPolicy}
          />
        </div>
        <div className="mainBottom row">
          <div className="col-3">
            <BalancerChoice balancerPolicy={balancerPolicy} />
          </div>
          <div className="col-5">
            <NetworkView workerContext={workerContext} wsclient={websocketclient} />
          </div>
          <div className="col-1">
            <IdleTime workerContext={workerContext} wsclient={websocketclient} />
          </div>
          <div className="col-3">
            <NodeProgress workerContext={workerContext} wsClient={websocketclient} />
          </div>
        </div>
      </div>
    );
  }
}

ReactDOM.render(<App />, document.getElementById("root") as HTMLElement);
registerServiceWorker();
