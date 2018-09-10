import Point from "../misc/Point";
import { tileSize } from "./Constants";
import { getBottomRightPoint, getTopLeftPoint } from "./Project";
import WebSocketClient, { RegionData } from "../connection/WSClient";
import { Map as LeafletMap } from "leaflet";
import RegionOfInterest from "./RegionOfInterest";

export default class MatrixView {

  /**
   * These variables handle the current visible region bounds as points
   */
  public topLeft: Point;
  public bottomRight: Point;
  private callbacks: Map<string, (roi: RegionOfInterest) => any>;

  constructor(tileDisplay: any, webSocketClient: WebSocketClient) {
    /*
    * this map stores callbacks to render all the tiles requested for leaflet.
    */
    this.callbacks = new Map();

    const handleRegionData = (msg: RegionData) => {
      const region = msg.workerInfo.region;
      const zoom = region.validation;

      // compute x/y coordinates based on region
      const regionTileSize = region.guaranteedDivisor;
      const xEnd = region.width / regionTileSize;
      const yEnd = region.height / regionTileSize;

      const topLeft = new Point(
        this.topLeft.x + region.hOffset / regionTileSize,
        this.topLeft.y - region.vOffset / regionTileSize,
        zoom,
      );
      topLeft.y *= -1;

      // and invoke tile draw methods
      for (let y = 0; y < yEnd; y++) {
        for (let x = 0; x < xEnd; x++) {
          const tileX = topLeft.x + x;
          const tileY = topLeft.y + y;
          const render = this.callbacks.get(coordsToString(tileX, tileY, zoom));
          if (render === undefined || render === null) {
            console.error(
              'Region not found for ' + new Point(tileX, tileY, zoom)
            );
            continue;
          }
          // only pass data of this region
          const realX = x * tileSize;
          const realY = y * tileSize;
          const tl = new Point(realX, realY);
          const br = new Point(realX + tileSize, realY + tileSize);

          const roi = new RegionOfInterest(
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

    const handleNewView = (map: LeafletMap) => {
      const bounds = map.getPixelBounds();
      const zoom = map.getZoom();
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
  public registerTile(origin: Point, draw: (data: RegionOfInterest) => any) {
    let promise;
    const render = (data: RegionOfInterest) => {
      promise = new Promise((resolve, error) => {
        try {
          draw(data);
          resolve();
        } catch (err) {
          error(err);
        }
      });
    };
    const coords = coordsToString(origin.x, origin.y, origin.z);
    this.callbacks.set(coords, render);
    return promise;
  }
}

function coordsToString(x: number, y: number, z: number): string {
  return [x, y, z].join(", ");
}
