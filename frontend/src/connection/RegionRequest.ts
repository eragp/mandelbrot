import { getBottomRightPoint, getTopLeftPoint, project } from "../tileDisplay/Project";
import { Request } from "../connection/ExchangeTypes";
import { TileSize } from "../Constants";
// import { Point3D } from "../misc/Point";
import { Map } from "leaflet";
import { Point3D } from "../misc/Point";
import { notDeepEqual } from "assert";

let oldTopLeft: Point3D;
let oldBottomRight: Point3D;
let oldBalancer: string;
let oldFractal: string;
let oldNodes: number;
let oldMaxIteration: number;
let oldPredictionAccuracy: number;
let oldRun: number;

/**
 *  Sends a region request for the currently visible region
 *
 * @param {*} map current Leaflet map
 */
export const request = (
  map: Map,
  balancer: string,
  fractal: string,
  nodes: number,
  maxIteration: number,
  predictionAccuracy: number,
  run: number
): Request | null => {
  const bounds = map.getPixelBounds();
  const zoom = map.getZoom();

  const topLeft = getTopLeftPoint(bounds, TileSize, zoom);
  const botRight = getBottomRightPoint(bounds, TileSize, zoom);

  const tlComplex = project(topLeft.x, topLeft.y, topLeft.z, 0, 0, TileSize);
  const brComplex = project(botRight.x, botRight.y, botRight.z, 0, 0, TileSize);
  const [sizeX, sizeY] = [
    Math.abs(botRight.x - topLeft.x) * TileSize,
    Math.abs(topLeft.y - botRight.y) * TileSize
  ];
  if (
    topLeft.equals(oldTopLeft) &&
    botRight.equals(oldBottomRight) &&
    balancer === oldBalancer &&
    fractal === oldFractal &&
    nodes === oldNodes &&
    maxIteration === oldMaxIteration &&
    predictionAccuracy === oldPredictionAccuracy &&
    run === oldRun
  ) {
    return null;
  }
  oldTopLeft = topLeft;
  oldBottomRight = botRight;
  oldBalancer = balancer;
  oldFractal = fractal;
  oldNodes = nodes;
  oldMaxIteration = maxIteration;
  oldPredictionAccuracy = predictionAccuracy;
  oldRun = run;

  const region = {
    type: "regionRequest",
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
      guaranteedDivisor: TileSize,
      maxIteration,
      fractal,
      regionCount: 0
    },
    predictionAccuracy,
    balancer,
    fractal,
    nodes
  };
  return region;
};
