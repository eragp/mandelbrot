import React, { Component } from 'react';
// leaflet stuff
import 'leaflet/dist/leaflet.css';
import L from 'leaflet';
// custom stylesheet
import './TileDisplay.css';
import './MandelbrotLayer';

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
    createTile: function(coords) {
      // var error;
      // var tile = L.DomUtil.create('canvas', 'leaflet-tile');
      var tile = document.createElement('div');
      const fromLatLng = map.unproject(coords, coords.z);
      const toLatLng = map.unproject(
        L.point(coords.x + 1, coords.y + 1),
        coords.z
      );
      var size = this.getTileSize();
      tile.width = size.x;
      tile.height = size.y;
      var url =
        'http://localhost:8080/mandelbrot?' +
        'fromReal=' +
        fromLatLng.lng.toString() +
        '&fromImag=' +
        fromLatLng.lat.toString() +
        '&toReal=' +
        toLatLng.lng.toString() +
        '&toImag=' +
        toLatLng.lat.toString();
      console.log(url);
      tile.innerHTML =
        'from: ' + fromLatLng + '<br> to: ' + toLatLng + '<br>' + url;
      tile.style.outline = '1px solid red';
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
  const scale = 100;
  L.gridLayer.mandelBrotLayer = () =>
    new L.GridLayer.MandelbrotLayer({
      tileSize: 256, // in px
      bounds: [[-2 * scale, -2 * scale], [2 * scale, 2 * scale]]
    });
  var map = L.map('viewer', {
    crs: L.CRS.Simple,
    minZoom: 1,
    center: [0, 0],
    zoom: 1
  });
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

export default TileDisplay;
