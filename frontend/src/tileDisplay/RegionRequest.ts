import { getBottomRightPoint, getTopLeftPoint, project } from "./Project";
import { tileSize, maxIteration } from "./Constants";
import 3DPoint from "../misc/Point";
import { Map } from "leaflet";

// making sure only new requests actually get sent
let currentTopLeft: 3DPoint;
let currentBottomRight: 3DPoint;
let currentBalancer: string;
/**
 *  Sends a region request for the currently visible region
 *
 * If the region has not changed from the last request, null is returned.
 * Otherwise the corresponding request for the backend is returned.
 * @param {*} map current Leaflet map
 */
export const request = (map: Map, balancer: string) => {
  const bounds = map.getPixelBounds();
  const zoom = map.getZoom();

  const topLeft = getTopLeftPoint(bounds, tileSize, zoom);
  const botRight = getBottomRightPoint(bounds, tileSize, zoom);

  // has the visible region changed?
  if (
    topLeft.equals(currentTopLeft) &&
    botRight.equals(currentBottomRight) &&
    currentBalancer === balancer
  ) {
    return;
  }
  currentTopLeft = topLeft;
  currentBottomRight = botRight;
  currentBalancer = balancer;

  const tlComplex = project(topLeft.x, topLeft.y, topLeft.z, 0, 0, tileSize);
  const brComplex = project(botRight.x, botRight.y, botRight.z, 0, 0, tileSize);
  const [sizeX, sizeY] = [
    Math.abs(botRight.x - topLeft.x) * tileSize,
    Math.abs(topLeft.y - botRight.y) * tileSize,
  ];
  const region = {
    region: {
      // point top left
      minReal: tlComplex.x,
      maxImag: tlComplex.y,
      // point top right
      maxReal: brComplex.x,
      minImag: brComplex.y,
      // computed region size
      width: sizeX,
      height: sizeY,
      // Super region => no offset
      hOffset: 0,
      vOffset: 0,
      // region identification via zoom factor
      validation: zoom,
      // Divisor for width and height. Will be used to perform load balancing
      guaranteedDivisor: tileSize,
      maxIteration,
    },
    type: "regionRequest",
    balancer,
  };
  // console.log("sending Region request: ");
  // console.log(region);
  return region;
};
