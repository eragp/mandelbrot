import "bootstrap/dist/css/bootstrap.css";

import * as React from "react";
import * as ReactDOM from "react-dom";

import TileDisplay from "./tileDisplay/TileDisplay";
import WebSocketClient from "./connection/WSClient";
import {
  BalancerObservable,
  ImplementationObservable,
  GroupObservable,
  ViewCenterObservable,
  WorkerObservable,
  IterationObservable,
  PredAccObservable,
  RunObservable
} from "./misc/Observable";
import { readViewCenterParams, readWSUrl } from "./misc/URLParams";

// Custom Components
import ComputationTime from "./components/ComputationTime";
import NetworkView from "./components/NetworkView";
import IdleTime from "./components/IdleTime";
import SelectBox from "./components/SelectBox";

// CSS
import "./index.css";
// Bootstrap
import registerServiceWorker from "./registerServiceWorker";
import { StatsCollector } from "./eval/StatsCollector";

class App extends React.Component<{}, {}> {
  render() {
    const ws = new WebSocketClient(readWSUrl());
    const stats = new StatsCollector(ws);

    const balancer = new BalancerObservable();
    const group = new GroupObservable();
    const impl = new ImplementationObservable();
    const iter = new IterationObservable();
    const viewCenter = new ViewCenterObservable();
    const workerCount = new WorkerObservable();
    const predAcc = new PredAccObservable();
    const run = new RunObservable();

    viewCenter.set(readViewCenterParams());

    return (
      <div className="index">
        <div className="mainTop">
          <TileDisplay
            group={group}
            wsclient={ws}
            balancer={balancer}
            implementation={impl}
            iterationCount={iter}
            viewCenter={viewCenter}
            workerCount={workerCount}
            stats={stats}
            predAcc={predAcc}
            run={run}
          />
        </div>
        <div className="mainBottom row">
          <div className="col-3">
            <SelectBox balancer={balancer} implementation={impl} />
          </div>
          <div className="col">
            <NetworkView group={group} wsclient={ws} />
          </div>
          <div className="col-1">
            <IdleTime group={group} wsclient={ws} />
          </div>
          <div className="col-3">
            <ComputationTime group={group} wsClient={ws} />
          </div>
        </div>
      </div>
    );
  }
}

ReactDOM.render(<App />, document.getElementById("root") as HTMLElement);
registerServiceWorker();
