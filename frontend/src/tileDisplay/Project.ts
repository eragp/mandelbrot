import 3DPoint from "../misc/Point";
import { bounds, leafletBound } from "./Constants";
import { Bounds } from "leaflet";

/**
 * This function projects leaflet tile coordinates to the complex plane
 * @param {*} tileX tile x position
 * @param {*} tileY tile y position
 * @param {*} zoom current zoom factor
 * @param {*} pixelX pixel x within the tile in [0, size]
 * @param {*} pixelY pixel y within the tile in [0, size]
 * @param {*} tileSize pixel dimensions of a tile (tiles have to be square)
 */
export const project = (tileX: number, tileY: number, zoom: number, 
  pixelX: number, pixelY: number, tileSize: number): 3DPoint => {
  tileSize = tileSize || 1;
  // top left -> bottom right
  // bounds in the imaginary plane have to be symmetric
  const tileCount = Math.pow(2, zoom) * 16;
  const real = (tileX * bounds[0]) / tileCount;
  const imag = (tileY * bounds[1]) / tileCount;
  return new 3DPoint(real, imag);
};

/**
 * Unprojects from the complex plane to leaflet tile coordinates
 * @param {number} real Real part of the complex coordinate
 * @param {number} imag Imaginary part of the complex coordinate
 * @param {number} zoom current zoom factor in leaflet space
 * @returns {3DPoint} Leaflet tile coordinate corresponding to real/imag coordinate
 */
export const unproject = (real: number, imag: number, zoom: number): 3DPoint => {
  const tileCount = Math.pow(2, zoom) * 16;
  const x = (tileCount * real) / bounds[0];
  const y = (tileCount * imag) / bounds[1];
  return new 3DPoint(Math.floor(x), Math.floor(y), zoom);
};

/**
 * Converts a complex coordinate to a leaflet CRS coordinate
 * @param {number} real real coordinate on the complex plane
 * @param {number} imag imaginary coordinate on the complex plane
 * @param {number} zoom zoom factor
 * @returns {3DPoint} projected point
 */
export const complexToLeaflet = (real: number, imag: number, zoom: number): 3DPoint => {
  return new 3DPoint(
    (imag * leafletBound) / bounds[1],
    (real * leafletBound) / bounds[0],
    zoom,
  );
};

/**
 * Converts a leaflet CRS coordinate to a complex coordinate
 * @param {number} lat  Latitude in CRS Space
 * @param {number} lng  Longitude in CRS Space
 * @param {number} zoom  zoom factor
 * @returns {3DPoint} projected point
 */
export const leafletToComplex = (lat: number, lng: number, zoom: number): 3DPoint => {
  return new 3DPoint(
    (lng / leafletBound) * bounds[1],
    (lat / leafletBound) * bounds[0],
    zoom,
  );
};

/**
 * calculates the TopLeft point in leaflet coordinates from the given bounds
 * @param {Bounds} bounds pixel bounds of the current view
 * @param {number} tileSize leaflet tile size
 * @param {number} zoom zoom factor
 */
export const getTopLeftPoint = (curBounds: Bounds, tileSize: number, zoom: number): 3DPoint => {
  if (curBounds.min !== undefined){
    return toPoint(curBounds.min, tileSize, zoom, true);
  }
  throw new TypeError("Bounds minimum is undefined");
};

/**
 * calculates the TopLeft point in leaflet coordinates from the given bounds
 * @param {Bounds} bounds pixel bounds of the current view
 * @param {Number} tileSize leaflet tile size
 * @param {Number} zoom zoom factor
 */
export const getBottomRightPoint = (curBounds: Bounds, tileSize: number, zoom: number): 3DPoint => {
  if (curBounds.max !== undefined){
    return toPoint(curBounds.max, tileSize, zoom, false);
  }
  throw new TypeError("Bounds maximum is undefined");
};

/**
 *
 * @param {3DPoint} bound
 * @param {Number} tileSize
 * @param {Number} zoom
 * @param {Boolean} topLeft
 */
function toPoint(bound: L.Point, tileSize: number, zoom: number, topLeft: boolean) {
  let x: number;
  let y: number;
  if (topLeft) {
    x = Math.floor(bound.x / tileSize);
  } else {
    x = Math.ceil(bound.x / tileSize);
  }
  if (topLeft) {
    y = Math.floor(bound.y / tileSize);
  } else {
    y = Math.ceil(bound.y / tileSize);
  }
  return new 3DPoint(x, -y, zoom);
}
