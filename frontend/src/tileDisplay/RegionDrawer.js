import { registerWorker, registerRegion } from '../connection/WSClient';

/*
 * this map stores callbacks to render all the tiles requested for leaflet.
 */
const callbacks = new Map();

const handleRegionData = msg => {
    // TODO compute x/y coordinates based on region
    // and invoke tile draw methods
};

/**
 * Register yourself at WSClient as regionData observer
 */
registerWorker(handleRegionData);


/**
 *  Registers the tile at coords to be drawn as soon as data is available.
 * @param {*} point coordinates on the tile to be registerd
 * @param {*} draw function expecting data that draws the tile @coords
 */
export const register = (point, draw) => {
    let promise;
    const render = data => {
      promise = new Promise((resolve, error) => {
        try {
          draw(data);
          resolve();
        } catch (err) {
          error(err);
        }
      });
    };
    let coords = coordsToString(point.x, point.y, point.z);
    callbacks.set(coords, render);
    return promise;
  };

function coordsToString(x, y, z) {
    return [x, y, z].join(', ');
  }