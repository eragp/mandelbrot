import Point from '../misc/Point';

const tileSize = 256;
const balancer = 'naive';

// making sure only new requests actually get sent
var oldTl = null;
var oldBr = null;
/**
 *  Sends a region request for the currently visible region
 *
 * If the region has not changed from the last request, null is returned.
 * Otherwise the corresponding request for the backend is returned.
 * @param {*} map current Leaflet map
 */
export const request = map => {
  let bounds = map.getPixelBounds();
  let zoom = map.getZoom();
  // aka top left
  let topLeft = new Point(
    Math.floor(bounds.min.x / tileSize),
    -Math.floor(bounds.min.y / tileSize),
    zoom
  );
  // aka bottom right
  let botRight = new Point(
    Math.floor(bounds.max.x / tileSize),
    -Math.floor(bounds.max.y / tileSize),
    zoom
  );
  if (topLeft.equals(oldTl) && botRight.equals(oldBr)) {
    return null;
  }
  oldTl = topLeft;
  oldBr = botRight;

  let min = unproject(topLeft.x, topLeft.y, topLeft.z, 0, 0, tileSize);
  let max = unproject(botRight.x, botRight.y, botRight.z, 0, 0, tileSize);
  console.log(
    'region Request on complex plane: ' +
      unproject(topLeft.x, topLeft.y, topLeft.z, 0, 0, tileSize) +
      ' -> ' +
      unproject(botRight.x, botRight.y, botRight.z, 0, 0, tileSize)
  );
  let region = {
    zoom: zoom,
    tlX: topLeft.x,
    tlY: topLeft.y,
    brX: botRight.x,
    brY: botRight.y,
    balancer: balancer
  };
  // console.log('sending request: ');
  // console.log(region);
  // let region = {
  //   zoom: zoom,
  //   minReal: min.x,
  //   maxReal: max.x,
  //   minImag: min.y,
  //   maxImag: max.y,
  //   balancer: balancer
  // };
  return region;
};

/**
 * This function unprojects leaflet tile coordinates to CRS Space
 * @param {*} x tile x position
 * @param {*} y tile y position
 * @param {*} zoom current zoom factor
 * @param {*} localX pixel x within the tile in [0, size]
 * @param {*} localY pixel y within the tile in [0, size]
 * @param {*} size pixel dimensions of a tile (tiles have to be square)
 */
export const unproject = (x, y, zoom, localX, localY, size) => {
  size = size || 1;
  // top left -> bottom right
  // bounds in the imaginary plane have to be symmetric
  let bounds = [4, 4];
  let tileCount = Math.pow(2, zoom);
  let tileX = (x * bounds[0] * size + localX * bounds[0]) / (tileCount * size),
    tileY = (y * bounds[1] * size + localY * bounds[1]) / (tileCount * size);
  return new Point(tileX, tileY);
};
