import Point from '../misc/Point.ts';
import { bounds, leafletBound } from './Constants';

/**
 * This function projects leaflet tile coordinates to the complex plane
 * @param {*} tileX tile x position
 * @param {*} tileY tile y position
 * @param {*} zoom current zoom factor
 * @param {*} pixelX pixel x within the tile in [0, size]
 * @param {*} pixelY pixel y within the tile in [0, size]
 * @param {*} tileSize pixel dimensions of a tile (tiles have to be square)
 */
export const project = (tileX, tileY, zoom, pixelX, pixelY, tileSize) => {
  tileSize = tileSize || 1;
  // top left -> bottom right
  // bounds in the imaginary plane have to be symmetric
  let tileCount = Math.pow(2, zoom) * 16;
  let real = (tileX * bounds[0]) / tileCount,
    imag = (tileY * bounds[1]) / tileCount;
  return new Point(real, imag);
};

/**
 * Unprojects from the complex plane to leaflet tile coordinates
 * @param {Number} real Real part of the complex coordinate
 * @param {Number} imag Imaginary part of the complex coordinate
 * @param {Number} zoom current zoom factor in leaflet space
 * @returns {Point} Leaflet tile coordinate corresponding to real/imag coordinate
 */
export const unproject = (real, imag, zoom) => {
  let tileCount = Math.pow(2, zoom) * 16;
  let x = (tileCount * real) / bounds[0],
    y = (tileCount * imag) / bounds[1];
  return new Point(Math.floor(x), Math.floor(y), zoom);
};

/**
 * Converts a complex coordinate to a leaflet CRS coordinate
 * @param {Number} real real coordinate on the complex plane
 * @param {Number} imag imaginary coordinate on the complex plane
 * @param {Number} zoom zoom factor
 * @returns {Point} projected point
 */
export const complexToLeaflet = (real, imag, zoom) => {
  return new Point(
    (imag * leafletBound) / bounds[1],
    (real * leafletBound) / bounds[0],
    zoom
  );
};

/**
 * Converts a leaflet CRS coordinate to a complex coordinate
 * @param {Number} lat  Latitude in CRS Space
 * @param {Number} lng  Longitude in CRS Space
 * @param {Number} zoom  zoom factor
 * @returns {Point} projected point
 */
export const leafletToComplex = (lat, lng, zoom) => {
  return new Point(
    (lng / leafletBound) * bounds[1],
    (lat / leafletBound) * bounds[0],
    zoom
  );
};

/**
 * calculates the TopLeft point in leaflet coordinates from the given bounds
 * @param {Bounds} bounds pixel bounds of the current view
 * @param {Number} tileSize leaflet tile size
 * @param {Number} zoom zoom factor
 */
export const getTopLeftPoint = (bounds, tileSize, zoom) => {
  return toPoint(bounds.min, tileSize, zoom, true);
};

/**
 * calculates the TopLeft point in leaflet coordinates from the given bounds
 * @param {Bounds} bounds pixel bounds of the current view
 * @param {Number} tileSize leaflet tile size
 * @param {Number} zoom zoom factor
 */
export const getBottomRightPoint = (bounds, tileSize, zoom) => {
  return toPoint(bounds.max, tileSize, zoom, false);
};

/**
 *
 * @param {Bounds} bound
 * @param {Number} tileSize
 * @param {Number} zoom
 * @param {Boolean} topLeft
 */
function toPoint(bound, tileSize, zoom, topLeft) {
  let x, y;
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
  return new Point(x, -y, zoom);
}
