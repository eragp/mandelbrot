import React, { Component } from 'react';
import ReactDOM from 'react-dom';
// leaflet stuff
import 'leaflet/dist/leaflet.css';
import  L from 'leaflet/dist/leaflet-src.js';
import 'leaflet-zoombox';
import 'leaflet-zoombox/L.Control.ZoomBox.css';

// custom stylesheet
import './TileDisplay.css';

import Shader from './Shader';
import { request, unproject } from './RegionRequest';
import { register, sendRequest, registerRegion, registerWorker } from '../connection/WSClient';
import Point from '../misc/Point';

//Custom component
import NodeList from '../Node';

class TileDisplay extends Component {
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
    // convert wierd leaflet coordinates to something more sensible
    let p = new Point(coords.x, coords.y, zoom);

    const drawTile = tileData => {
      let ctx = tile.getContext('2d', { alpha: false });
      ctx.fillStyle = 'black';
      ctx.fillRect(0, 0, tile.width, tile.height);

      let imgData = ctx.createImageData(size.x, size.y);
      for (let y = 0; y < size.y; y++) {
        for (let x = 0; x < size.x; x++) {
          // TODO: investigate why this works for now it's a dirty hack.
          let n = tileData[tileData.length - (y + 1) * size.x + x];
          let [r, g, b] = Shader.default(n, 200);
          drawPixel(imgData, x, y, r, g, b, 255);
        }
      }

      // tile.style.outline = '1px solid red';
      ctx.putImageData(imgData, 0, 0);
      done(null, tile);
    };
    console.log(
      'requesting new tile at ' +
        p +
        ' complex: ' +
        unproject(p.x, p.y, zoom, 0, 0, 256)
    );
    register(p, drawTile);
    return tile;
  }
});

L.GridLayer.DebugLayer = L.GridLayer.extend({
  createTile: function(coords, done) {
    let tile = document.createElement('div');
    let size = this.getTileSize();
    let zoom = map.getZoom();
    tile.width = size.x;
    tile.height = size.y;
    // convert wierd leaflet coordinates to something more sensible
    let p = new Point(coords.x, coords.y, zoom);

    setTimeout(() => {
      tile.innerHTML = p.toString;
      tile.style.outline = '1px solid red';
      done(null, tile);
    }, 100);
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

var map = null;
function renderLeaflet() {
  // bounds have to be a power of two
  let bounds = [[-256, -256], [256,256]];
  L.gridLayer.mandelBrotLayer = () =>
    new L.GridLayer.MandelbrotLayer({
      tileSize: 256, // in px
      bounds: bounds,
      keepBuffer: 0
    });
  L.gridLayer.debugLayer = () => 
    new L.GridLayer.DebugLayer({
      tileSize: 256,
      bounds: bounds,
      keepBuffer: 0
    });
  map = L.map('viewer', {
    crs: L.CRS.Simple,
    // maxZoom: 32,
    zoom: 3
  });
  const requestCallback = () => {
    let r = request(map);
    if (r !== null) {
      sendRequest(r);
    }
  };
  map.on({
    moveend: requestCallback
  });
  // add event listeners to the map for region requests
  let layer = L.gridLayer.mandelBrotLayer();
  map.addLayer(layer);
  map.setView([0, 0]);

  var legend = L.control({position: 'topright'});
  let nodeList;

  legend.onAdd = function (map) {
      let container = L.DomUtil.create('div', 'nodeControl');
      ReactDOM.render(<NodeList ref={(component) => {nodeList = component;}}/>, container);;
      return container;
  };
  legend.addTo(map);

  registerRegion((data) => {
    // Draw node progress
    let nworkers = data.nregions;
    console.log(data)
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

  // TODO register workers at websocket client
  // so that they are set inactive when the first tile/region
  // by them comes in
  registerWorker((data) => {
    // Stop corresponding worker progress bar
    // assume that regionData is passed here
    let workerID = data.rank;
    nodeList.setState((oldState) => {
      // Pay attention here that ranks begin from 1 as long as the host does not send data on his own
      oldState.active[workerID-1] = false;
      // TODO insert correct µs time in node value
      return oldState;
    });
  });

}



export default TileDisplay;
