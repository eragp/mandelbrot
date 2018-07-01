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
import { request } from './RegionRequest';
import { sendRequest } from '../connection/WSClient';
import { register } from './RegionDrawer';
import { tileSize } from './Constants';
import Point from '../misc/Point';

/**
 * Functions to call, when a new region is issued
 */
const newRegionObservers = [];

const registerNewRegion = callback => {
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
  newRegionObservers.push(fun);
  return promise;
};

export default class TileDisplay extends Component {
  componentDidMount() {
    renderLeaflet();
  }

  render() {
    return <div id="viewer" />;
  }
}

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
    const drawTile = tileData => {
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
    register(p, drawTile);
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

var map = null;
function renderLeaflet() {
  // bounds have to be a power of two
  // these bounds are chosen arbitrary and have nothing to do with
  // either leaflet space, nor the complex plane
  let bounds = [[-256, -256], [256, 256]];
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
  map = L.map('viewer', {
    crs: L.CRS.Simple,
    // maxZoom: 32,
    zoom: 3
  });

  const requestCallback = map => {
    let r = request(map);
    if (r !== null) {
      sendRequest(r);
    }
  };
  registerNewRegion(requestCallback);

  // add event listeners to the map for region requests
  map.on({
    moveend: () => {
      newRegionObservers.forEach(callback => callback(map));
    }
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
}

// export default TileDisplay;
