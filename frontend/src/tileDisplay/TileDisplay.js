import React, { Component } from 'react';
// leaflet stuff
import 'leaflet/dist/leaflet.css';
import L from 'leaflet';
// custom stylesheet
import './TileDisplay.css';

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
      // var tile = document.createElement('canvas');
      // var tile = document.createElement('div');
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
        mode: 'cors'
      })
        .then(response => response.json())
        .then(json => {
          // console.log(json);
          let ctx = tile.getContext('2d');
          ctx.clearRect(0, 0, size.x, size.y);
          for (let x = 0; x < size.x; x++) {
            for (let y = 0; y < size.y; y++) {
              let n = json[x + y * size.x];
              let r = (n * 10) % 256,
                g = (n * 20) % 256,
                b = (n * 40) % 256;
              ctx.fillStyle = 'rgba(' + r + ',' + g + ',' + b + ',' + 255 + ')';
              // let value = (x | y) % 256;
              // ctx.fillStyle =
              //   'rgba(' + value + ',' + value + ',' + value + ',' + 255 + ')';
              ctx.fillRect(x, y, 1, 1);
            }
          }
          tile.style.outline = '1px solid red';
          done(null, tile);
        })
        .catch(error => {
          console.error(error);
          done(error, tile);
        });
      // console.log(map.layerPointToLatLng(coords));
      // draw a moire pattern to the tile
      // let ctx = tile.getContext('2d');
      // ctx.clearRect(0, 0, size.x, size.y);
      // // let px = ctx.createImageData(1, 1);
      // for (let x = 0; x < size.x; x += 4) {
      //   for (let y = 0; y < size.y; y += 4) {
      //     let value = x | y % 256;
      //     ctx.fillStyle =
      //       'rgba(' + value + ',' + value + ',' + value + ',' + 255 + ')';
      //     ctx.fillRect(x, y, 4, 4);
      //   }
      // }
      return tile;
    }
  });
  // bounds have to be a power of two
  let bounds = [[-256, -256], [256, 256]];
  L.gridLayer.mandelBrotLayer = () =>
    new L.GridLayer.MandelbrotLayer({
      tileSize: 256, // in px
      bounds: bounds
    });

  var map = L.map('viewer', {
    crs: L.CRS.Simple,
    // minZoom: 0,
    center: [0, 0],
    zoom: 0
  });
  // map.fitBounds(bounds);
  // L.marker(L.latLng([0, 0])).addTo(map);
  map.addLayer(L.gridLayer.mandelBrotLayer());
  // L.geoJSON(generate(), { style: styleUsageMap }).addTo(map);
};

// const styleUsageMap = feature => {
//   return {
//     fillColor: getColor(feature.properties.time),
//     weight: 1,
//     opacity: 1,
//     color: 'white',
//     dashArray: '3',
//     fillOpacity: 0.5
//   };
// };

// const getColor = value => {
//   return value > 1000
//     ? '#800026'
//     : value > 500
//       ? '#BD0026'
//       : value > 200
//         ? '#E31A1C'
//         : value > 100
//           ? '#FC4E2A'
//           : value > 50
//             ? '#FD8D3C'
//             : value > 20
//               ? '#FEB24C'
//               : value > 10
//                 ? '#FED976'
//                 : '#FFEDA0';
// };

const unproject = (x, y, zoom, localX, localY, size) => {
  // top left -> bottom right
  // bounds in the imaginary plane have to be symmetric
  size = size || 1;
  let bounds = [3, 2];
  let tileCount = Math.pow(2, zoom);
  let tileX = (x * bounds[0] * size + localX * bounds[0]) / (tileCount * size),
    tileY = -(y * bounds[1] * size + localY * bounds[1]) / (tileCount * size);
  return new Point(tileX, tileY);
};

class Point {
  constructor(x, y) {
    this.x = x;
    this.y = y;
  }
  toString() {
    return 'Point{' + this.x.toString() + ', ' + this.y.toString() + '}';
  }
}

export default TileDisplay;
