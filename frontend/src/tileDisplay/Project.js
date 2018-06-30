import Point from '../misc/Point';
import { bounds } from './Constants';

/**
 * This function projects leaflet tile coordinates to the complex plane
 * @param {*} tileX tile x position
 * @param {*} tileY tile y position
 * @param {*} zoom current zoom factor
 * @param {*} pixelX pixel x within the tile in [0, size]
 * @param {*} pixelY pixel y within the tile in [0, size]
 * @param {*} pixelSize pixel dimensions of a tile (tiles have to be square)
 */
export const project = (tileX, tileY, zoom, pixelX, pixelY, pixelSize) => {
  pixelSize = pixelSize || 1;
  // top left -> bottom right
  // bounds in the imaginary plane have to be symmetric
  let tileCount = Math.pow(2, zoom);
  let real =
      (tileX * bounds[0] * pixelSize + pixelX * bounds[0]) /
      (tileCount * pixelSize),
    imag =
      -(tileY * bounds[1] * pixelSize + pixelY * bounds[1]) /
      (tileCount * pixelSize);
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
    y = -(tileCount * imag) / bounds[1];
  return new Point(Math.floor(x), Math.floor(y), zoom);
};
