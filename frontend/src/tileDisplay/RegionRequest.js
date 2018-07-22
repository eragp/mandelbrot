import { getBottomRightPoint, getTopLeftPoint, project } from './Project';
import { tileSize, balancer } from './Constants';

// making sure only new requests actually get sent
var currentTopLeft = null;
var currentBottomRight = null;
/**
 *  Sends a region request for the currently visible region
 *
 * If the region has not changed from the last request, null is returned.
 * Otherwise the corresponding request for the backend is returned.
 * @param {*} map current Leaflet map
 */
export const request = (map, balancerPolicy) => {
  let bounds = map.getPixelBounds();
  let zoom = map.getZoom();

  let topLeft = getTopLeftPoint(bounds, tileSize, zoom);
  let botRight = getBottomRightPoint(bounds, tileSize, zoom);

  // has the visible region changed?
  if (topLeft.equals(currentTopLeft) && botRight.equals(currentBottomRight)) {
    return null;
  }
  currentTopLeft = topLeft;
  currentBottomRight = botRight;

  let tlComplex = project(topLeft.x, topLeft.y, topLeft.z, 0, 0, tileSize);
  let brComplex = project(botRight.x, botRight.y, botRight.z, 0, 0, tileSize);
  let [sizeX, sizeY] = [
    (Math.abs(botRight.x - topLeft.x)) * tileSize,
    (Math.abs(topLeft.y - botRight.y)) * tileSize
  ];
  let region = {
    // point top left
    minReal: tlComplex.x,
    maxImag: tlComplex.y,
    // point top right
    maxReal: brComplex.x,
    minImag: brComplex.y,
    // computed region size
    width: sizeX,
    height: sizeY,
    // region identification via zoom factor
    validation: zoom,
    // Divisor for width and height. Will be used to perform load balancing
    guaranteedDivisor: tileSize,
    balancer: balancerPolicy.getBalancer(),
    maxIteration: 256,
  };
  console.log('sending Region request: ');
  console.log(region);
  return region;
};
