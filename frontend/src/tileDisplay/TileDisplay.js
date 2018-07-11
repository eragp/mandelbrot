import React, { Component } from 'react';
import ReactDOM from 'react-dom';
// leaflet stuff
import 'leaflet/dist/leaflet.css';
import L from 'leaflet/dist/leaflet-src.js';
import 'leaflet-zoombox';
import 'leaflet-zoombox/L.Control.ZoomBox.css';

// custom stylesheet
import './TileDisplay.css';

import Shader from './Shader';
import { project, unproject } from './Project';
import { request as requestRegion } from './RegionRequest';
import WebSocketClient from '../connection/WSClient';

import { tileSize } from './Constants';
import Point from '../misc/Point';
import RegionDrawer from './RegionDrawer';

//Custom component
import NodeList from '../Node';


export default class extends Component {
  componentDidMount() {
    this.map = null;
    /**
     * Functions to call, when a new region is issued
     */
    this.newViewObservers = [];
    this.websocketClient = new WebSocketClient();
    this.regionDrawer = new RegionDrawer(this, this.websocketClient);
    this.renderLeaflet();
  }

  renderLeaflet() {
    let websocketClient = this.websocketClient;
    let regionDrawer = this.regionDrawer;
    // bounds have to be a power of two
    // these bounds are chosen arbitrary and have nothing to do with
    // either leaflet space, nor the complex plane
    let bounds = [[-256, -256], [256, 256]];
    let map = L.map('viewer', {
      crs: L.CRS.Simple,
      // maxZoom: 32,
      zoom: 3
    });

    // Request a new region subdivision via websocket on view change
    let requestCallback = map => {
      let r = requestRegion(map);
      if (r !== null) {
        websocketClient.sendRequest(r);
      }
    };
    this.registerNewView(requestCallback);

    // add event listeners to the map for region requests

    map.on({
      moveend: () => {
        this.newViewObservers.forEach(callback => callback(map));
      }
    });

    function drawPixel(imgData, x, y, r, g, b) {
      let d = imgData.data;
      let i = (x << 2) + ((y * imgData.width) << 2);
      d[i] = r; // red
      d[i + 1] = g; // green
      d[i + 2] = b; // blue
      d[i + 3] = 255; // alpha
    }

 
    L.GridLayer.MandelbrotLayer = L.GridLayer.extend({
      createTile: function(coords, done) {
        let tile = L.DomUtil.create('canvas', 'leaflet-tile');
        let size = this.getTileSize();
        // when zooming map.getZoom() is not up to date
        // therefore the value has to read directly from the layer
        let zoom = this._tileZoom;
        tile.width = size.x;
        tile.height = size.y;
        let p = new Point(coords.x, coords.y, zoom);

        /**
         * Draw tile callback, asserts tileData to be RegionOfInterest object
         * (see RegionDrawer)
         */
        var drawTile = tileData => {
          let ctx = tile.getContext('2d', { alpha: false });
          ctx.fillStyle = 'black';
          ctx.fillRect(0, 0, tile.width, tile.height);

          let imgData = ctx.createImageData(size.x, size.y);
          for (let y = 0; y < size.y; y++) {
            for (let x = 0; x < size.x; x++) {
              let n = tileData.get(x, y);
              let [r, g, b] = Shader.default(n, 200);
              drawPixel(imgData, x, y, r, g, b, 255);
            }
          }

          ctx.putImageData(imgData, 0, 0);
          done(null, tile);
        };
        console.log(
          'requesting new tile at ' +
            p +
            ' complex: ' +
            project(p.x, p.y, zoom, 0, 0, tileSize)
        );
        regionDrawer.register(p, drawTile);
        return tile;
      }
    });

    L.GridLayer.DebugLayer = L.GridLayer.extend({
      createTile: function(coords) {
        let div = document.createElement('div');
        let size = this.getTileSize();
        let zoom = this._tileZoom;
        div.width = size.x;
        div.height = size.y;

        let p = new Point(coords.x, coords.y, zoom);

        let projected = project(coords.x, coords.y, zoom, 0, 0, tileSize);
        let unprojected = unproject(projected.x, projected.y, zoom);

        div.classList.add('debugLayer');
        div.innerHTML =
          'Leaflet tile: ' +
          p.toString() +
          '</br>Projected: ' +
          projected +
          '</br>Unprojected: ' +
          unprojected;
        return div;
      }
    });

    let mandelbrotLayer = new L.GridLayer.MandelbrotLayer({
        tileSize: tileSize, // in px
        bounds: bounds,
        keepBuffer: 0
      }),
      debugLayer = new L.GridLayer.DebugLayer({
        tileSize: tileSize,
        bounds: bounds,
        keepBuffer: 0
      });
    let baseLayer = {
        'Mandelbrot Layer': mandelbrotLayer
      },
      overlayLayers = {
        'Debug Layer': debugLayer
      };
    map.addLayer(mandelbrotLayer);

    L.control.layers(baseLayer, overlayLayers).addTo(map);
    map.setView([0, 0]);

    // Add node progress

    var legend = L.control({position: 'bottomleft'});
    let nodeList;
  
    legend.onAdd = function (map) {
        let container = L.DomUtil.create('div', 'nodeControl');
        ReactDOM.render(<NodeList ref={(component) => {nodeList = component;}}/>, container);;
        return container;
    };
    legend.addTo(map);

    // TODO move outside => into NodeList?

    // register workers at websocket client
    // so that they are set inactive when the first tile/region
    // by them comes in
    websocketClient.registerWorker((data) => {
      // Stop corresponding worker progress bar
      // assume that regionData is passed here
      let workerID = data.workerInfo.rank;
      nodeList.setState((oldState) => {
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
      nodeList.setState({
        numWorkers: nworkers,
        active: active,
        progress: progress
      });
      // TODO draw regions
    });
  
    map.addControl(
      L.control.zoomBox({
        modal: true,
        title: 'Box area zoom'
      })
    );

    this.map = map;
  }

  /**
   * Invoke the given callback, when the view of the map has changed
   */
  registerNewView(callback) {
    let promise;
    const fun = data => {
      promise = new Promise((resolve, error) => {
        try {
          resolve(callback(data));
        } catch (err) {
          error(err);
        }
      });
    };
    this.newViewObservers.push(fun);
    return promise;
  }

  render() {
    return <div id="viewer" />;
  }
}
