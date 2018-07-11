import React, { Component } from 'react';

export default class extends Component {

    componentDidMount() {

        let websocketClient = this.props.wsclient;
        // TODO move outside => into NodeList?

        // register workers at websocket client
        // so that they are set inactive when the first tile/region
        // by them comes in
        websocketClient.registerWorker((data) => {
            // Stop corresponding worker progress bar
            // assume that regionData is passed here
            let workerID = data.workerInfo.rank;
            this.setState((oldState) => {
                // Pay attention here that ranks begin from 1 as long as the host does not send data on his own
                oldState.active[workerID-1] = false;
                // TODO insert correct µs time in node value
                return oldState;
            });
        });
        
        websocketClient.registerRegion((data) => {
            // Draw node progress
            let nworkers = data.regionCount;
            let active = new Array(nworkers);
            let progress = new Array(nworkers);
            for(var i = 0; i < nworkers; i++){
                active[i] = true;
                progress[i] = 0;
            }
            this.setState({
            numWorkers: nworkers,
            active: active,
            progress: progress
            });
            // TODO draw regions
        });
    }

    render(){
        return <div>hello!</div>;
    }
    
}