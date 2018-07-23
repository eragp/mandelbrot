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
import workerSVG from './img/worker.svg';
import serverSVG from './img/server.svg';
import applicationSVG from './img/application.svg';

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
              }
            },
            layout: {
              hierarchical: {
                enabled: true,
                direction: 'LR'
              }
            }
        };

        let fakeNodes = new DataSet([{
            id: 0,
            label: 'Frontend',
            image: applicationSVG,
            shape: 'image'
        }, {
            id: 1,
            label: 'Backend-Host',
            image: serverSVG,
            shape: 'image'
        }, {
            id: 2,
            label: 'Worker 0',
            image: workerSVG,
            shape: 'image'
        }]);

        let fakeEdges = new DataSet([{
            from: 0,
            to: 1
        }, {
            from: 1,
            to: 2
        }]);

        this.network = new Network(
            document.getElementById("nodeNetwork"), {
                nodes: fakeNodes,
                edges: fakeEdges
            }, options
        );

        /**
         * Redraw graph when information about backend becomes available
         */
        this.props.wsclient.registerRegion(data => {
            let nodes = [];

            nodes.push({
                id: 0,
                label: 'Frontend',
                image: applicationSVG,
                shape: 'image'
            })
            nodes.push({
                id: 1,
                label: 'Backend-Host',
                image: serverSVG,
                shape: 'image'
            })

            let edges = [];
            edges.push({
                from: 0,
                to: 1
            })
            for (let region of data.regions) {
                nodes.push({
                    id: region.nodeID + 2,
                    label: `Worker ${region.nodeID}`,
                    image: workerSVG,
                    shape: 'image'
                })
                edges.push({
                    from: 1,
                    to: region.nodeID + 2
                })
            }

            this.network.setData({
              nodes: new DataSet(nodes),
              edges: new DataSet(edges)
            });
        });
    }

    render() {
        return (<div id="nodeNetwork"></div>);
    }
}

NetworkView.propTypes = {
    wsclient: PropTypes.instanceOf(WebSocketClient)
}