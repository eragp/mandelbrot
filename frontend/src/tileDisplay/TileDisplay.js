import React, { Component } from 'react';
// leaflet stuff
import 'leaflet/dist/leaflet.css';
import L from 'leaflet';
// custom stylesheet
import './TileDisplay.css';

import './Shader';
import Shader from './Shader';
import request from './RegionRequest';
import { register, render } from './Callback';

// overwrite fetch to add timeout functionality
// import fetch from '../misc/fetchTimeout';
class TileDisplay extends Component {
  componentDidMount() {
    renderLeaflet();
  }

  render() {
    return <div id="viewer" />;
  }
}

function renderLeaflet() {
  L.GridLayer.MandelbrotLayer = L.GridLayer.extend({
    createTile: function(coords, done) {
      var tile = L.DomUtil.create('canvas', 'leaflet-tile');
      var size = this.getTileSize();
      tile.width = size.x;
      tile.height = size.y;
      var url =
        'http://localhost:8080/mandelbrot?' +
        'x=' +
        coords.x +
        '&y=' +
        coords.y +
        '&z=' +
        coords.z +
        '&size=' +
        size.x;
      const drawTile = data => {
        let ctx = tile.getContext('2d', { alpha: false });
        ctx.fillStyle = 'black';
        ctx.fillRect(0, 0, tile.width, tile.height);

        let imgData = ctx.createImageData(size.x, size.y);
        for (let x = 0; x < size.x; x += 1) {
          for (let y = 0; y < size.y; y += 1) {
            let v = x ^ y;
            drawPixel(imgData, x, y, v, v, v, 255);
          }
        }
        tile.style.outline = '1px solid red';
        ctx.putImageData(imgData, 0, 0);
        done(null, tile);
      };
      register(coords, drawTile);
      return tile;
    }
  });

  function drawPixel(imgData, x, y, r, g, b, a) {
    let d = imgData.data;
    let i = (x << 2) + ((y * imgData.width) << 2);
    d[i] = r;
    d[i + 1] = g;
    d[i + 2] = b;
    d[i + 3] = a;
  }
  // bounds have to be a power of two
  let bounds = [[-256, -256], [256, 256]];
  L.gridLayer.mandelBrotLayer = () =>
    new L.GridLayer.MandelbrotLayer({
      tileSize: 256, // in px
      bounds: bounds,
      keepBuffer: 16
    });

  var map = L.map('viewer', {
    crs: L.CRS.Simple,
    // minZoom: 0,
    // otherwise we get precision errors.
    maxZoom: 32,
    // center: [0, 0],
    zoom: 3
  });
  map.on({
    // load: () => request(map),
    // move: () => request(map),
    // zoom: () => request(map),
    moveend: () => {
      request(map);
      render();
    }
  });
  // add event listeners to the map for region requests
  let layer = L.gridLayer.mandelBrotLayer();
  // layer.on({
  //   load: () => render()
  // });
  map.addLayer(layer);
  map.setView([0, 0]);
}

export default TileDisplay;
