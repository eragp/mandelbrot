import Point from '../misc/Point';
import { bounds } from './Constants';

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
  let tileCount = Math.pow(2, zoom);
  let real =
      (tileX * bounds[0] * tileSize + pixelX * bounds[0]) /
      (tileCount * tileSize),
    imag =
      (tileY * bounds[1] * tileSize + pixelY * bounds[1]) /
      (tileCount * tileSize);
  return new Point(real, imag);
};

/**
 * Unprojects from the complex plane to leaflet tile coordinates
 * @param {*} real Real part of the complex coordinate
 * @param {*} imag Imaginary part of the complex coordinate
 * @param {*} zoom current zoom factor in leaflet space
 */
export const unproject = (real, imag, zoom) => {
  let tileCount = Math.pow(2, zoom);
  let x = (tileCount * real) / bounds[0],
    y = (tileCount * imag) / bounds[1];
  return new Point(Math.floor(x), Math.floor(y), zoom);
};

/**
 * calculates the TopLeft point in leaflet coordinates from the given bounds
 * @param {*} bounds pixel bounds of the current view
 * @param {*} tileSize leaflet tile size
 * @param {*} zoom zoom factor
 */
export const getTopLeftPoint = (bounds, tileSize, zoom) => {
  return toPoint(bounds.min, tileSize, zoom, true);
};

/**
 * calculates the TopLeft point in leaflet coordinates from the given bounds
 * @param {*} bounds pixel bounds of the current view
 * @param {*} tileSize leaflet tile size
 * @param {*} zoom zoom factor
 */
export const getBottomRightPoint = (bounds, tileSize, zoom) => {
  return toPoint(bounds.max, tileSize, zoom, false);
};

function toPoint(bound, tileSize, zoom, topleft) {
  let x, y;
  if (topleft) {
    x = Math.floor(bound.x / tileSize);
  } else {
    x = Math.ceil(bound.x / tileSize);
  }
  if (topleft) {
    y = Math.floor(bound.y / tileSize);
  } else {
    y = Math.ceil(bound.y / tileSize);
  }
  return new Point(x, -y, zoom);
}
