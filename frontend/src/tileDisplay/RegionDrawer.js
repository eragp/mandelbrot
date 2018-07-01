import { registerWorker, registerRegion } from '../connection/WSClient';
import { registerNewRegion } from './TileDisplay';
// import { currentTopLeft, currentBottomRight } from './RegionRequest';
import Point from '../misc/Point';

/*
 * this map stores callbacks to render all the tiles requested for leaflet.
 */
const callbacks = new Map();
/**
 * These variables handle the current visible region bounds as points
 */
var topLeft;
var bottomRight;

/**
 * Registers the tile at coords to be drawn as soon as data is available.
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

const handleRegionData = msg => {
  let region = msg.workerInfo.region;
  let zoom = msg.validation;
  // compute x/y coordinates based on region
  let tileSize = region.guaranteedDivisor;
  let xStart = region.hOffset / tileSize;
  let xEnd = xStart + region.width / tileSize;
  let yStart = region.vOffset / tileSize;
  let yEnd = yStart + region.height / tileSize;

  // and invoke tile draw methods
  for (let y = yStart; y < yEnd; y++) {
    for (let x = xStart; x < xEnd; x++) {
      let tileX = topLeft.x + x;
      let tileY = topLeft.y + y;
      let cb = callbacks.get(coordsToString(tileX, tileY, zoom));
      if (cb === null) {
        console.log('Region not found for ' + new Point(x, y, zoom));
        continue;
      }
      // only pass data of this region
      // TODO test
      let realX = x * tileSize;
      let realY = y * tileSize;
      let tl = new Point(realX, realY);
      let br = new Point(realX + tileSize, realY + tileSize);
      let roi = new RegionOfInterest(
        tl,
        br,
        msg.data,
        region.width,
        region.height
      );
      cb(roi);
    }
  }
};

const handleNewRegion = map => {
  let bounds = map.getPixelBounds();
  let zoom = map.getZoom();
  let tileSize = map.getTileSize();
  // aka top left
  topLeft = new Point(
    Math.floor(bounds.min.x / tileSize),
    -Math.floor(bounds.min.y / tileSize),
    zoom
  );
  // aka bottom right
  bottomRight = new Point(
    Math.floor(bounds.max.x / tileSize),
    -Math.floor(bounds.max.y / tileSize),
    zoom
  );
};

/**
 * Register yourself at WSClient as regionData observer
 */
registerWorker(handleRegionData);

/**
 * Also observe the current region
 */
registerNewRegion(handleNewRegion);

/**
 * Represents a view on the data of the rectangle defined by tl, br.
 */
class RegionOfInterest {
  /**
   * @param {*} tl TopLeft Point
   * @param {*} br BottomRight Point
   * @param {*} data data for super region
   * @param {*} width width of the super region
   * @param {*} height height of the super region
   */
  constructor(tl, br, data, width, height) {
    this.topLeft = tl;
    this.bottomRight = br;
    this.data = data;
    this.width = width;
    this.height = height;

    this.ROIWidth = topLeft.x - bottomRight.x;
    this.ROIHeight = topLeft.y - bottomRight.y;
  }

  /**
   * Returns the correct value of the underlying data array
   * by wrapping x and y in the rectangle of topLeft, bottomRight.
   */
  get(x, y) {
    if (x > this.ROIWidth || y > this.ROIHeight) {
      console.log('Illegal access');
      return -1;
    }
    let realX = this.topLeft.x + x;
    let realY = this.topLeft.y + y;
    return this.data[realY * this.width + realX];
  }
}

function coordsToString(x, y, z) {
  return [x, y, z].join(', ');
}
