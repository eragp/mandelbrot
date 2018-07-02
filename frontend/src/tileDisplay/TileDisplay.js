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
import WebSocketClient from '../connection/WSClient';

import { tileSize } from './Constants';
import Point from '../misc/Point';
import RegionDrawer from './RegionDrawer';





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
    let websocketClient = this.websocketClient;
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

    let regionDrawer = this.regionDrawer;
    L.GridLayer.MandelbrotLayer = L.GridLayer.extend({
      createTile: function(coords, done) {
        let tile = L.DomUtil.create('canvas', 'leaflet-tile');
        let size = this.getTileSize();
        let zoom = map.getZoom();
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
    
    function drawPixel(imgData, x, y, r, g, b) {
      let d = imgData.data;
      let i = (x << 2) + ((y * imgData.width) << 2);
      d[i] = r; // red
      d[i + 1] = g; // green
      d[i + 2] = b; // blue
      d[i + 3] = 255; // alpha
    }
    
    L.GridLayer.DebugLayer = L.GridLayer.extend({
      createTile: function(coords) {
        let tile = document.createElement('div');
        let size = this.getTileSize();
        let zoom = map.getZoom();
        tile.width = size.x;
        tile.height = size.y;
    
        let p = new Point(coords.x, coords.y, zoom);
    
        let projected = project(coords.x, coords.y, zoom, 0, 0, tileSize);
        let unprojected = unproject(projected.x, projected.y, zoom);
        tile.innerHTML =
          'Leaflet tile: ' +
          p.toString() +
          '</br>Projected: ' +
          projected +
          '</br>Unprojected: ' +
          unprojected;
        tile.style.outline = '1px solid red';
        return tile;
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
    map.addLayer(debugLayer);
    L.control.layers(baseLayer, overlayLayers).addTo(map);
    map.setView([0, 0]);
    this.map = map;
  }

  /**
   * Invoke the given callback, when the view of the map has changed
   */
  registerNewView(callback){
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
  };

  render() {
    return <div id="viewer" />;
  }
}




