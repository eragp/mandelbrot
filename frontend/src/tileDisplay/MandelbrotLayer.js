import 'leaflet/dist/leaflet.css';
import L from 'leaflet';

L.GridLayer.MandelbrotLayer = L.GridLayer.extend({
  createTile: function(coords) {
    // var error;
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
    console.log(coords);
    // draw a moire pattern to the tile
    let ctx = tile.getContext('2d');
    ctx.clearRect(0, 0, size.x, size.y);
    let px = ctx.createImageData(1, 1);
    for (let x = 0; x < size.x; x++) {
      for (let y = 0; y < size.y; y++) {
        let value = (x * y) & 0xff;
        ctx.fillStyle =
          'rgba(' + value + ',' + value + ',' + value + ',' + 255 + ')';
        ctx.fillRect(x, y, 1, 1);
      }
    }
    return tile;
  }
});

// const renderTile = async () => {
//   console.log('Requesting: ' + url);
//   try {
//     let response = await fetch(url, { method: 'GET', mode: 'cors' });
//     let json = await response.json();
//     console.log(json);
//     for (let y = 0; y < size; y++) {
//       for (let x = 0; x < size; x++) {
//         let n = json[x + y * size];
//         let r = (n * 10) % 256;
//         let g = (n * 20) % 256;
//         let b = (n * 40) % 256;
//         ctx.fillStyle = 'rgba(' + r + ',' + g + ',' + b + ', 255)';
//         ctx.fillRect(x, y, x, y);
//       }
//     }
//   } catch (exception) {
//     console.error(exception);
//   }
// };
// renderTile();
//drawing the shape of the point
//adding gradient
