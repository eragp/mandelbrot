import React, { Component } from 'react';
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

import { tileSize } from './Constants';
import Point from '../misc/Point';
import RegionDrawer from './RegionDrawer';
import BalancerPolicy from '../misc/BalancerPolicy';
import WebSocketClient from '../connection/WSClient';
import PropTypes from 'prop-types';
import WorkerLayer from './WorkerLayer';
import WorkerContext from '../misc/WorkerContext';


export default class TileDisplay extends Component {

  componentDidMount() {
    this.map = null;
    /**
     * Functions to call, when a new region is issued
     */
    this.newViewObservers = [];
    this.websocketClient = this.props.wsclient;
    this.balancerPolicy = this.props.balancerPolicy;
    this.workerContext = this.props.workerContext;
    this.regionDrawer = new RegionDrawer(this, this.websocketClient);
    this.renderLeaflet();
  }

  renderLeaflet() {
    // bounds have to be a power of two
    // these bounds are chosen arbitrary and have nothing to do with
    // either leaflet space, nor the complex plane
    let bounds = [[-256, -256], [256, 256]];
    this.map = L.map('viewer', {
      crs: L.CRS.Simple,
      // maxZoom: 32,
      zoom: 3
    });

    const map = this.map;
    const websocketClient = this.websocketClient;
    const regionDrawer = this.regionDrawer;

    // Request a new region subdivision via websocket on view change
    this.registerNewView(map => {
      let r = requestRegion(map, this.balancerPolicy.getBalancer());
      if (r !== null) {
        websocketClient.sendRequest(r);
      }
    });

    // Handle balancer change as view change 
    //  => update all view subscribers about a policy change as if the view had changed
    this.balancerPolicy.subscribe(() => this.updateAllNewView());

    // add event listeners to the map for region requests

    map.on({
      moveend: () => this.updateAllNewView()
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
        /*console.log(
          'requesting new tile at ' +
            p +
            ' complex: ' +
            project(p.x, p.y, zoom, 0, 0, tileSize)
        );*/
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

    const mandelbrotLayer = new L.GridLayer.MandelbrotLayer({
        tileSize: tileSize, // in px
        bounds: bounds,
        keepBuffer: 0
      });
    const debugLayer = new L.GridLayer.DebugLayer({
        tileSize: tileSize,
        bounds: bounds,
        keepBuffer: 0
      });
    const workerLayer = new WorkerLayer(
      websocketClient,
      map.unproject.bind(map),
      this.workerContext
    );
    const baseLayer = {
        'Mandelbrot Layer': mandelbrotLayer
      };
    const overlayLayers = {
        'Debug Layer': debugLayer,
        'Worker Layer': workerLayer
      };
    map.addLayer(mandelbrotLayer);
    map.addLayer(workerLayer);

    L.control.layers(baseLayer, overlayLayers).addTo(map);
    map.setView([0, 0]);

    map.addControl(
      L.control.zoomBox({
        modal: true,
        title: 'Box area zoom'
      })
    );

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

  updateAllNewView(){
    this.newViewObservers.forEach(callback => callback(this.map));
  }

  render() {
    return <div id="viewer" />;
  }
}

TileDisplay.propTypes = {
  wsclient: PropTypes.instanceOf(WebSocketClient),
  balancerPolicy: PropTypes.instanceOf(BalancerPolicy),
  workerContext: PropTypes.instanceOf(WorkerContext)
}
