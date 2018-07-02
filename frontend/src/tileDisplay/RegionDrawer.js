import Point from '../misc/Point';
import { tileSize } from './Constants';
import { unproject } from './Project';

export default class {
  constructor(tileDisplay, webSocketClient) {
    /*
    * this map stores callbacks to render all the tiles requested for leaflet.
    */
    this.callbacks = new Map();
    /**
     * These variables handle the current visible region bounds as points
     */
    this.topLeft;
    this.bottomRight;

    let handleRegionData = msg => {
      let region = msg.workerInfo.region;
      let zoom = region.validation;
      // compute x/y coordinates based on region
      let tileSize = region.guaranteedDivisor;
      let xEnd = region.width / tileSize;
      let yEnd = region.height / tileSize;

      let topLeft = unproject(region.minReal, region.maxImag, zoom);

      // and invoke tile draw methods
      for (let y = 0; y < yEnd; y++) {
        for (let x = 0; x < xEnd; x++) {
          let tileX = topLeft.x + x;
          let tileY = topLeft.y + y;
          let cb = this.callbacks.get(coordsToString(tileX, tileY, zoom));
          if (cb === undefined || cb === null) {
            console.log(
              'Region not found for ' + new Point(tileX, tileY, zoom)
            );
            continue;
          }
          // only pass data of this region
          // TODO test
          let realX = x * tileSize;
          let realY = y * tileSize;
          let tl = new Point(realX, realY);
          let br = new Point(realX + tileSize, realY + tileSize);

          console.log('current tile ' + tileX + ', ' + tileY);
          let roi = new RegionOfInterest(
            tl,
            br,
            msg.data,
            region.width,
            region.height
            // region.width,
            // region.height
          );
          cb(roi);
        }
      }
    };

    let handleNewRegion = map => {
      let bounds = map.getPixelBounds();
      let zoom = map.getZoom();
      let tileSize = tileSize;
      // aka top left
      this.topLeft = new Point(
        Math.floor(bounds.min.x / tileSize),
        -Math.floor(bounds.min.y / tileSize),
        zoom
      );
      // aka bottom right
      this.bottomRight = new Point(
        Math.floor(bounds.max.x / tileSize),
        -Math.floor(bounds.max.y / tileSize),
        zoom
      );
    };
    /**
     * Register yourself at WSClient as regionData observer
     */
    webSocketClient.registerWorker(handleRegionData);

    /**
     * Also observe the current region
     */
    tileDisplay.registerNewView(handleNewRegion);
  }

  /**
   * Registers the tile at coords to be drawn as soon as data is available.
   * @param {*} point coordinates on the tile to be registerd
   * @param {*} draw function expecting data that draws the tile @coords
   */
  register(point, draw) {
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
    this.callbacks.set(coords, render);
    return promise;
  }
}

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

    this.ROIWidth = br.x - tl.x;
    this.ROIHeight = br.y - tl.y;
  }

  /**
   * Returns the correct value of the underlying data array
   * by wrapping x and y in the rectangle of topLeft, bottomRight.
   */
  get(x, y) {
    if (x > this.ROIWidth || y > this.ROIHeight) {
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
