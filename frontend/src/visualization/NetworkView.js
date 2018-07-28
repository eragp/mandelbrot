import React, {
    Component
} from 'react';
import PropTypes from 'prop-types';
import WebSocketClient from '../connection/WSClient';

import {
    DataSet,
    Network
} from 'vis';

import './NetworkView.css';
// TODO include material design icon copyright notice
// TODO add white filled circle behind the icons so that edges don't collide with them (effectively get covered)
import workerImage from './img/worker.backgroundCircle.svg';
import serverImage from './img/server.backgroundCircle.svg';
import applicationImage from './img/application.backgroundCircle.svg';

export default class NetworkView extends Component {

    componentDidMount() {
        const options = {
            autoResize: true,
            physics: {
              enabled: false
            },
            edges: {
              smooth: {
                enabled: false
              },
              color: {
                  color: 'black'
              }
            },
            layout: {
              hierarchical: {
                enabled: true,
                direction: 'LR',
                sortMethod: 'directed'
              }
            },
            interaction: {
                dragNodes: false,
                dragView: false,
                hover: true,
            }
        };

        this.network = new Network(
            document.getElementById("nodeNetwork"), {
                nodes: new DataSet(),
                edges: new DataSet()
            }, options
        );

        this.networkState = {
            numWorkers: 1
        };
        this.renderNetwork();

        /**
         * Redraw graph when information about backend becomes available
         */
        this.props.wsclient.registerRegion(data => {
            const newWorkers = data.regionCount;
            if(this.networkState.numWorkers !== newWorkers){
                this.networkState.numWorkers = newWorkers;
                this.renderNetwork();
            }
        });
    }

    /**
     * Updates the Dataset of the network according to the received number of workers
     */
    renderNetwork(){
        let nodes = [];

            nodes.push({
                id: 0,
                label: 'Frontend',
                image: applicationImage,
                shape: 'image'
            })
            nodes.push({
                id: 1,
                label: 'Backend-Host',
                image: serverImage,
                shape: 'image'
            })

            let edges = [];
            edges.push({
                from: 0,
                to: 1
            })
            for (let id = 0; id < this.networkState.numWorkers; id += 1) {
                nodes.push({
                    id: id + 2,
                    label: `Worker ${id}`,
                    image: workerImage,
                    shape: 'image'
                })
                edges.push({
                    from: 1,
                    to: id + 2
                })
            }

            this.network.setData({
              nodes: new DataSet(nodes),
              edges: new DataSet(edges)
            });

            this.network.fit();
    }

    render() {
        return (<div id="nodeNetwork"></div>);
    }
}

NetworkView.propTypes = {
    wsclient: PropTypes.instanceOf(WebSocketClient)
}