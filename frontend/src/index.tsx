import "bootstrap/dist/css/bootstrap.css";

import * as React from "react";
import * as ReactDOM from "react-dom";

import TileDisplay from "./tileDisplay/TileDisplay";

import WebSocketClient from "./connection/WSClient";
import { getURLParams } from "./misc/URLParams";

// Custom Components
import NodeProgress from "./components/NodeProgress";
import NetworkView from "./components/NetworkView";
import IdleTime from "./components/IdleTime";
import SelectBox from "./components/SelectBox";

import { BalancerObservable, ImplementationObservable, GroupObservable } from "./misc/Observable";

// CSS
import "./index.css";
// Bootstrap
import registerServiceWorker from "./registerServiceWorker";

class App extends React.Component<{}, {}> {
  render() {
    const websocketclient = new WebSocketClient();
    const balancer = new BalancerObservable();
    const group = new GroupObservable();
    const impl = new ImplementationObservable();

    return (
      <div className="index">
        <div className="mainTop">
          <TileDisplay
            group={group}
            wsclient={websocketclient}
            balancer={balancer}
            implementation={impl}
            viewCenter={getURLParams()}
          />
        </div>
        <div className="mainBottom row">
          <div className="col-3">
            <SelectBox balancer={balancer} implementation={impl} />
          </div>
          <div className="col">
            <NetworkView group={group} wsclient={websocketclient} />
          </div>
          <div className="col-1">
            <IdleTime group={group} wsclient={websocketclient} />
          </div>
          <div className="col-3">
            <NodeProgress group={group} wsClient={websocketclient} />
          </div>
        </div>
      </div>
    );
  }
}

ReactDOM.render(<App />, document.getElementById("root") as HTMLElement);
registerServiceWorker();
