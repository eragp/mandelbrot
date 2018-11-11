import * as React from "react";
// leaflet stuff
import "leaflet/dist/leaflet.css";
import * as L from "leaflet/dist/leaflet-src.js";
import "leaflet-zoombox";
import "leaflet-zoombox/L.Control.ZoomBox.css";
import { Map } from "leaflet";

// custom stylesheet
import "./TileDisplay.css";

import Shader from "./Shader";
import { project, unproject } from "./Project";
import { request as requestRegion } from "./RegionRequest";

import { tileSize, leafletBound } from "./Constants";
import Point from "../misc/Point";
import MatrixView from "./MatrixView";
import BalancerPolicy from "../misc/BalancerPolicy";
import WebSocketClient from "../connection/WSClient";
import WorkerLayer from "./WorkerLayer";
import WorkerContext from "../misc/WorkerContext";
import { setURLParams } from "../misc/URLParams";
import RegionOfInterest from "./RegionOfInterest";

interface TileDisplayProps {
  wsclient: WebSocketClient;
  balancerPolicy: BalancerPolicy;
  workerContext: WorkerContext;
  viewCenter: Point;
}

export default class TileDisplay extends React.Component<TileDisplayProps, {}> {

  private map: Map;
  private newViewObservers: Array<(map: Map) => any>;
  private regionDrawer: MatrixView;
  private viewCenter: Point;

  /**
   * Invoke the given callback, when the view of the map has changed
   */
  public registerNewView(callback: (data: Map) => any) {
    let promise;
    const fun = (data: Map) => {
      promise = new Promise((resolve, error) => {
        try {
          resolve(callback(data));
        } catch (err) {
          error(err);
        }
      });
    };
    this.newViewObservers.push(fun);
    return promise;
  }

  public render() {
    return <div id="viewer" />;
  }

  public componentDidMount() {
    /**
     * Functions to call, when a new region is issued
     */
    this.newViewObservers = [];
    this.regionDrawer = new MatrixView(this, this.props.wsclient);

    this.renderLeaflet();
  }

  private renderLeaflet() {
    // bounds have to be a power of two
    // these bounds are chosen arbitrary and have nothing to do with
    // either leaflet space, nor the complex plane
    const bounds = [[-leafletBound, -leafletBound], [leafletBound, leafletBound]];
    this.map = L.map("viewer", {
      crs: L.CRS.Simple,
      maxZoom: 50,
      zoom: 0,
      // bounds preventing the user from panning outside the mandelbrot set
      maxBounds: bounds,
      maxBoundsViscosity: 1
    });

    const map = this.map;
    const websocketClient = this.props.wsclient;
    const regionDrawer = this.regionDrawer;

    // Request a new region subdivision via websocket on view change
    this.registerNewView((curMap: Map) => {
      const r = requestRegion(curMap, this.props.balancerPolicy.getBalancer());
      if (r !== undefined) {
        websocketClient.sendRequest(r);
      }
    });

    // Handle balancer change as view change
    //  => update all view subscribers about a policy change as if the view had changed
    this.props.balancerPolicy.subscribe(
      () => this.updateAllViews(),
    );

    // add event listeners to the map for region requests
    map.on({
      moveend: () => this.updateAllViews(),
    });

    function drawPixel(imgData: ImageData, x: number, y: number, r: number, g: number, b: number, alpha: number) {
      const d = imgData.data;
      const i = (x << 2) + ((y * imgData.width) << 2);
      d[i] = r; // red
      d[i + 1] = g; // green
      d[i + 2] = b; // blue
      d[i + 3] = alpha || 255; // alpha
    }

    L.GridLayer.MandelbrotLayer = L.GridLayer.extend({

      createTile(coords: Point, done: (e: Error | null, t: HTMLCanvasElement) => any) {
        const tile = L.DomUtil.create("canvas", "leaflet-tile") as HTMLCanvasElement;
        const size = this.getTileSize();
        // when zooming map.getZoom() is not up to date
        // therefore the value has to be read directly from the layer
        const zoom = this._tileZoom;
        tile.width = size.x;
        tile.height = size.y;
        const p = new Point(coords.x, coords.y, zoom);

        /**
         * Draw tile callback, asserts tileData to be RegionOfInterest object
         * (see RegionDrawer)
         */
        const drawTile = (tileData: RegionOfInterest) => {
          const ctx = tile.getContext("2d", { alpha: false });
          if (!ctx) {
            return;
          }
          ctx.fillStyle = "black";
          ctx.fillRect(0, 0, tile.width, tile.height);

          const imgData = ctx.createImageData(size.x, size.y);
          for (let y = 0; y < size.y; y++) {
            for (let x = 0; x < size.x; x++) {
              const n = tileData.get(x, y);
              const [r, g, b] = Shader.default(n, 200);
              drawPixel(imgData, x, y, r, g, b, 255);
            }
          }

          ctx.putImageData(imgData, 0, 0);
          done(null, tile);
        };
        regionDrawer.registerTile(p, drawTile);
        return tile;
      },
    });

    L.GridLayer.DebugLayer = L.GridLayer.extend({

      createTile(coords: Point) {
        const div = document.createElement("div");
        const size = this.getTileSize();
        const zoom = this._tileZoom;
        div.style.width = size.x;
        div.style.height = size.y;

        const p = new Point(coords.x, coords.y, zoom);

        const projected = project(coords.x, coords.y, zoom, 0, 0, tileSize);
        const unprojected = unproject(projected.x, projected.y, zoom);

        div.classList.add("debugLayer");
        div.innerHTML =
          "Leaflet tile: " +
          p.toString() +
          "</br>Projected: " +
          projected +
          "</br>Unprojected: " +
          unprojected;

        return div;
      },
    });

    const mandelbrotLayer = new L.GridLayer.MandelbrotLayer({
      tileSize: tileSize, // in px
      bound: bounds,
    });
    const debugLayer = new L.GridLayer.DebugLayer({
      tileSize: tileSize,
      bounds: bounds,
    });
    const workerLayer = new WorkerLayer(
      websocketClient,
      map.unproject.bind(map),
      this.props.workerContext,
    );
    const baseLayer = {
      "Mandelbrot Layer": mandelbrotLayer,
    };
    const overlayLayers = {
      "Debug Layer": debugLayer,
      "Worker Layer": workerLayer,
    };
    map.addLayer(mandelbrotLayer);
    map.addLayer(workerLayer);

    L.control.layers(baseLayer, overlayLayers).addTo(map);

    this.viewCenter = this.props.viewCenter;
    map.setView([this.viewCenter.x, this.viewCenter.y], this.viewCenter.z);
    // change URL params when region changes
    this.registerNewView((curMap: Map) => {
      const center = curMap.getCenter();
      const zoom = curMap.getZoom();
      this.viewCenter = new Point(center.lat, center.lng, zoom);
      setURLParams(this.viewCenter);
    });

    map.addControl(
      L.control.zoomBox({
        modal: true,
        title: "Box area zoom",
      }),
    );
  }

  private updateAllViews() {
    this.newViewObservers.forEach((callback) => callback(this.map));
  }

}
