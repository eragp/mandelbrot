import React, { Component } from 'react';
// leaflet stuff
import 'leaflet/dist/leaflet.css';
import  L from 'leaflet/dist/leaflet-src.js';
import 'leaflet-zoombox';
import 'leaflet-zoombox/L.Control.ZoomBox.css';

// custom stylesheet
import './TileDisplay.css';

import Shader from './Shader';

class TileDisplay extends Component {
  componentDidMount() {
    renderLeaflet();
  }

  render() {
    return <div id="viewer" />;
  }
}


const renderLeaflet = () => {
  L.GridLayer.MandelbrotLayer = L.GridLayer.extend({
    createTile: function(coords, done) {
      var tile = L.DomUtil.create('canvas', 'leaflet-tile');
      var size = this.getTileSize();
      tile.width = size.x;
      tile.height = size.y;
      var url =
        'http://localhost:8080/mandelbrot?x=' +
        coords.x +
        '&y=' +
        coords.y +
        '&z=' +
        coords.z +
        '&size=' +
        size.x;
      fetch(url, {
        method: 'GET',
        mode: 'cors',
        timeout: 1500
      })
        .then(response => response.json())
        .then(json => {
          let rank = json['rank'];
          let tile_values = json['tile'];
          // console.log(json);
          let ctx = tile.getContext('2d');
          ctx.clearRect(0, 0, size.x, size.y);
          for (let x = 0; x < size.x; x++) {
            for (let y = 0; y < size.y; y++) {
              let [r, g, b] = Shader.default(tile_values[x + y * size.x], 200);
              ctx.fillStyle = 'rgba(' + r + ',' + g + ',' + b + ',' + 255 + ')';
              ctx.fillRect(x, y, 1, 1);
            }
          }
          // tile.style.outline = '1px solid red';
          done(null, tile);
        })
        .catch(error => {
          console.error(error);
          done(error, tile);
        });
      return tile;
    }
  });
  // bounds have to be a power of two
  let bounds = [[-256, -256], [256, 256]];
  L.gridLayer.mandelBrotLayer = () =>
    new L.GridLayer.MandelbrotLayer({
      tileSize: 256, // in px
      bounds: bounds,
      keepBuffer: 16,
    });

  var map = L.map('viewer', {
    crs: L.CRS.Simple,
    // minZoom: 0,
    // otherwise we get precision errors.
    maxZoom: 30,
    center: [0, 0],
    zoom: 0
  });
  map.addLayer(L.gridLayer.mandelBrotLayer());

  // Add zoombox controls
  var options = {
    modal: true,
    title: "Box area zoom"
  };
  var control = L.control.zoomBox(options);
  map.addControl(control);
};

// const unproject = (x, y, zoom, localX, localY, size) => {
//   // top left -> bottom right
//   // bounds in the imaginary plane have to be symmetric
//   size = size || 1;
//   let bounds = [3, 2];
//   let tileCount = Math.pow(2, zoom);
//   let tileX = (x * bounds[0] * size + localX * bounds[0]) / (tileCount * size),
//     tileY = -(y * bounds[1] * size + localY * bounds[1]) / (tileCount * size);
//   return new Point(tileX, tileY);
// };

// class Point {
//   constructor(x, y) {
//     this.x = x;
//     this.y = y;
//   }
//   toString() {
//     return 'Point{' + this.x.toString() + ', ' + this.y.toString() + '}';
//   }
// }

export default TileDisplay;
