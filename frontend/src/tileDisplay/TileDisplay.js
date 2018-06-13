import React, { Component } from 'react';
// leaflet stuff
import 'leaflet/dist/leaflet.css';
import L from 'leaflet';
// custom stylesheet
import './TileDisplay.css';

import './Shader';
import Shader from './Shader';
import request from './RegionRequest';

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
          // console.error(error);
          done(error, tile);
        });
      return tile;
    }
  });
  // bounds have to be a power of two
  let bounds = [[-500, -500], [500,500]];
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
    center: [0, 0],
    zoom: 0
  });
  map.addLayer(L.gridLayer.mandelBrotLayer());

  // Create region requests on map changes
  map.on({
    zoom: () => request(map),
    moveend: () => request(map)
  });
};

export default TileDisplay;
