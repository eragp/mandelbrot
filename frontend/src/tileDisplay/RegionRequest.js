import Point from '../misc/Point';
import { project } from './Project';

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
  // has the visible region changed?
  if (topLeft.equals(oldTl) && botRight.equals(oldBr)) {
    return null;
  }
  oldTl = topLeft;
  oldBr = botRight;

  let min = project(topLeft.x, topLeft.y, topLeft.z, 0, 0, tileSize);
  let max = project(botRight.x, botRight.y, botRight.z, 0, 0, tileSize);
  console.log('region Request on complex plane: ' + min + ' -> ' + max);
  let region = {
    zoom: zoom,
    // point top left
    minReal: min.x,
    maxImag: max.y,
    // point top right
    maxReal: max.x,
    minImag: min.y,
    balancer: balancer
  };
  console.log('sending request: ');
  console.log(region);
  return region;
};
