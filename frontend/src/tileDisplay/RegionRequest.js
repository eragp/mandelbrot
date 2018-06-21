import 'leaflet/dist/leaflet.css';
import Point from '../misc/Point';

const tileSize = 256;
const balancer = 'naive';

function regionRequest(map) {
  let bounds = map.getPixelBounds();
  let zoom = map.getZoom();
  // aka top left
  let tl = new Point(
    Math.floor(bounds.min.x / tileSize),
    -Math.floor(bounds.min.y / tileSize),
    zoom
  );
  // aka bottom right
  let br = new Point(
    Math.floor(bounds.max.x / tileSize),
    -Math.floor(bounds.max.y / tileSize),
    zoom
  );
  console.log('region request from: ' + tl + ' to: ' + br);
  console.log(
    'complex plane: ' +
      unproject(tl.x, tl.y, tl.z, 0, 0, tileSize) +
      ' to: ' +
      unproject(br.x, br.y, br.z, 0, 0, tileSize)
  );
  let url =
    'http://localhost:8080/region?' +
    'zoom=' +
    zoom +
    '&topLeftX=' +
    tl.x +
    '&topLeftY=' +
    tl.y +
    '&bottomRightX=' +
    br.x +
    '&bottomRightY=' +
    (br.y - 1) +
    '&balancer=' +
    balancer;
  console.log(url);
  fetch(url, {
    method: 'GET',
    mode: 'cors',
    timeout: 1500
  })
    .then(response => response.json())
    .then(json => {
      console.log(json);
    })
    .catch(error => {
      console.log(error);
    });
}

/**
 * This function unprojects leaflet tile coordinates to CRS Space
 * @param {*} x tile x position
 * @param {*} y tile y position
 * @param {*} zoom current zoom factor
 * @param {*} localX pixel x within the tile in [0, size]
 * @param {*} localY pixel y within the tile in [0, size]
 * @param {*} size pixel dimensions of a tile (tiles have to be square)
 */
function unproject(x, y, zoom, localX, localY, size) {
  size = size || 1;
  // top left -> bottom right
  // bounds in the imaginary plane have to be symmetric
  let bounds = [4, 4];
  let tileCount = Math.pow(2, zoom);
  let tileX = (x * bounds[0] * size + localX * bounds[0]) / (tileCount * size),
    tileY = (y * bounds[1] * size + localY * bounds[1]) / (tileCount * size);
  return new Point(tileX, tileY);
}

export default regionRequest;
