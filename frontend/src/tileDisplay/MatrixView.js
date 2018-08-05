import Point from '../misc/Point';
import { tileSize } from './Constants';
import { getBottomRightPoint, getTopLeftPoint } from './Project';

export default class MatrixView {
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

      let topLeft = new Point(
        this.topLeft.x + region.hOffset / tileSize,
        this.topLeft.y - region.vOffset / tileSize,
        zoom
      );
      topLeft.y *= -1;

      // and invoke tile draw methods
      for (let y = 0; y < yEnd; y++) {
        for (let x = 0; x < xEnd; x++) {
          let tileX = topLeft.x + x;
          let tileY = topLeft.y + y;
          let render = this.callbacks.get(coordsToString(tileX, tileY, zoom));
          if (render === undefined || render === null) {
            console.error(
              'Region not found for ' + new Point(tileX, tileY, zoom)
            );
            continue;
          }
          // only pass data of this region
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
          render(roi);
        }
      }
    };

    /**
     * Register yourself at WSClient as regionData observer
     */
    webSocketClient.registerRegionData(handleRegionData);

    let handleNewView = map => {
      let bounds = map.getPixelBounds();
      let zoom = map.getZoom();
      this.topLeft = getTopLeftPoint(bounds, tileSize, zoom);
      this.bottomRight = getBottomRightPoint(bounds, tileSize, zoom);
    };

    /**
     * Also observe the current region
     */
    tileDisplay.registerNewView(handleNewView);
  }

  /**
   * Registers the tile at coords to be drawn as soon as data is available.
   * @param {Point} origin coordinates on the tile to be registerd
   * @param {*} draw function expecting data that draws the tile @coords
   */
  registerTile(origin, draw) {
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
    let coords = coordsToString(origin.x, origin.y, origin.z);
    this.callbacks.set(coords, render);
    return promise;
  }
}

/**
 * Represents a view on the data of the rectangle defined by tl, br.
 */
class RegionOfInterest {
  /**
   * @param {Point} tl TopLeft Point
   * @param {Point} br BottomRight Point
   * @param {*} data data for super region
   * @param {Number} width width of the super region
   * @param {Number} height height of the super region
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
   * @param {Number} x pixel x coordinate of the tile
   * @param {Number} y pixel y coordinate of the tile
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
