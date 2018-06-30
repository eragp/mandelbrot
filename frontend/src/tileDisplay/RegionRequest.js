import Point from '../misc/Point';
import { project } from './Project';
import { tileSize, balancer } from './Constants';


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

  let tlComplex = project(topLeft.x, topLeft.y, topLeft.z, 0, 0, tileSize);
  let brComplex = project(
    botRight.x + 1,
    botRight.y - 1,
    botRight.z,
    0,
    0,
    tileSize
  );
  let size = new Point(
    (Math.abs(botRight.x - topLeft.x) + 1) * tileSize,
    (Math.abs(topLeft.y - botRight.y) + 1) * tileSize
  );
  let region = {
    // point top left
    minReal: tlComplex.x,
    maxImag: brComplex.y,
    // point top right
    maxReal: brComplex.x,
    minImag: tlComplex.y,
    // computed region size
    width: size.x,
    height: size.y,
    // region identification via zoom factor
    validation: zoom,
    // Divisor for width and height. Will be used to perform load balancing
    guaranteedDivisor: tileSize,
    balancer: balancer,
    maxIteration: 100
  };
  console.log('sending Region request: ');
  console.log(region);
  return region;
};
