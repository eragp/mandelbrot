import 'leaflet/dist/leaflet.css';
import L from 'leaflet';

function regionRequest(map) {
  let bounds = map.getBounds();
  // aka top right
  let tr = mapToComplexPlane(bounds._northEast.lng, bounds._northEast.lat);
  // aka bottom left
  let bl = mapToComplexPlane(bounds._southWest.lng, bounds._southWest.lat);
  console.log('region request from: ' + bl + ' to: ' + tr);
  let url =
    'http://localhost:8080/mandelbrot/region' +
    '&fromR=' +
    bl.x +
    '&fromI=' +
    bl.y +
    '&toR=' +
    tr.x +
    '&toI=' +
    tr.y;
  fetch({
    url: url,
    method: 'GET',
    timeout: 1500
  })
    // .then(response => response.json())
    .catch(error => {
      console.log(error);
    });
}

/**
 * maps form CRS space to the complex Plane
 * @param {*} lng Longitude position in CRS.Simple
 * @param {*} lat Latitude position in CRS.Simple
 */
function mapToComplexPlane(lng, lat) {
  return new Point(lng / 1e2, lat / 1e2);
}

// /**
//  * This function unprojets the current leaflet
//  * @param {*} x tile x position
//  * @param {*} y tile y position
//  * @param {*} zoom current zoom factor
//  * @param {*} localX pixel x within the tile in [0, size]
//  * @param {*} localY pixel y within the tile in [0, size]
//  * @param {*} size pixel dimensions of a tile (tiles have to be square)
//  */
// function unproject(x, y, zoom, localX, localY, size) {
//   // top left -> bottom right
//   // bounds in the imaginary plane have to be symmetric
//   size = size || 1;
//   let bounds = [3, 2];
//   let tileCount = Math.pow(2, zoom);
//   let tileX = (x * bounds[0] * size + localX * bounds[0]) / (tileCount * size),
//     tileY = -(y * bounds[1] * size + localY * bounds[1]) / (tileCount * size);
//   return new Point(tileX, tileY);
// }

/**
 * Models a 2D Point
 */
class Point {
  constructor(x, y) {
    this.x = x;
    this.y = y;
  }
  toString() {
    return 'Point{' + this.x.toString() + ', ' + this.y.toString() + '}';
  }
}

export default regionRequest;
