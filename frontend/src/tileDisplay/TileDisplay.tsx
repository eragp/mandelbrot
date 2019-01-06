import * as React from "react";
// leaflet stuff
import "leaflet/dist/leaflet.css";
import * as L from "leaflet/dist/leaflet-src.js";
import "leaflet-zoombox";
import "leaflet-zoombox/L.Control.ZoomBox.css";
import { Map } from "leaflet";

import Shader from "./Shader";
import { project, unproject, complexToLeaflet, leafletToComplex } from "./Project";
import { request as requestRegion } from "../connection/RegionRequest";

import { TileSize, LeafletBound } from "../Constants";
import { Point3D } from "../misc/Point";
import MatrixView from "./MatrixView";
import WebSocketClient from "../connection/WSClient";
import WorkerLayer from "./WorkerLayer";
import { setURLParams } from "../misc/URLParams";
import RegionOfInterest from "./RegionOfInterest";
import {
  BalancerObservable,
  GroupObservable,
  ImplementationObservable,
  ViewCenterObservable
} from "../misc/Observable";
import { registerCallback } from "../misc/registerCallback";
import { StatsCollector } from "../eval/StatsCollector";

// custom stylesheet
import "./TileDisplay.css";

interface TileDisplayProps {
  wsclient: WebSocketClient;
  balancer: BalancerObservable;
  group: GroupObservable;
  implementation: ImplementationObservable;
  viewCenter: ViewCenterObservable;
  stats?: StatsCollector;
}
export default class TileDisplay extends React.Component<TileDisplayProps, {}> {
  private map: Map;
  private newViewObservers: Array<(map: Map) => any>;
  private regionDrawer: MatrixView;
  private center: Point3D;

  /**
   * Invoke the given callback, when the view of the map has changed
   */
  public registerNewView(fun: (data: Map) => any) {
    return registerCallback(this.newViewObservers, fun);
  }

  public render() {
    return <div id="viewer" />;
  }

  public componentDidMount() {
    this.newViewObservers = [];
    this.regionDrawer = new MatrixView(this, this.props.wsclient);

    this.renderLeaflet();
  }

  private renderLeaflet() {
    // bounds have to be a power of two
    // these bounds are chosen arbitrary and have nothing to do with
    // either leaflet space, nor the complex plane
    const bounds = [[-LeafletBound, -LeafletBound], [LeafletBound, LeafletBound]];
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
    const stats = this.props.stats;

    // Request a new region subdivision via websocket on view change
    this.registerNewView((curMap: Map) => {
      const r = requestRegion(curMap, this.props.balancer.get(), this.props.implementation.get());
      if (r) {
        websocketClient.sendRequest(r);
      }
    });

    // Handle balancer change as view change
    //  => update all view subscribers about a policy change as if the view had changed
    this.props.balancer.subscribe(() => this.updateAllViews());
    this.props.implementation.subscribe(() => this.updateAllViews());

    // add event listeners to the map for region requests
    map.on({
      moveend: () => this.updateAllViews()
    });

    function drawPixel(
      imgData: ImageData,
      x: number,
      y: number,
      r: number,
      g: number,
      b: number,
      alpha: number
    ) {
      const d = imgData.data;
      const i = (x << 2) + ((y * imgData.width) << 2);
      d[i] = r; // red
      d[i + 1] = g; // green
      d[i + 2] = b; // blue
      d[i + 3] = alpha || 255; // alpha
    }

    L.GridLayer.MandelbrotLayer = L.GridLayer.extend({
      createTile(coords: Point3D, done: (e: Error | null, t: HTMLCanvasElement) => any) {
        const tile = L.DomUtil.create("canvas", "leaflet-tile") as HTMLCanvasElement;
        const size = this.getTileSize();
        // when zooming map.getZoom() is not up to date
        // therefore the value has to be read directly from the layer
        const zoom = this._tileZoom;
        tile.width = size.x;
        tile.height = size.y;
        const p = new Point3D(coords.x, coords.y, zoom);
        /**
         * Draw tile callback, asserts tileData to be RegionOfInterest object
         * (see RegionDrawer)
         */
        const drawTile = (tileData: RegionOfInterest) => {
          // start timer
          const t0 = performance.now();

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

          // end timer
          const t1 = performance.now();
          if (stats) {
            stats.setDrawTime(tileData.rank , (t1 - t0) * 1000);
          }

          done(null, tile);
        };
        regionDrawer.registerTile(p, drawTile);
        return tile;
      }
    });

    L.GridLayer.DebugLayer = L.GridLayer.extend({
      createTile(coords: Point3D) {
        const div = document.createElement("div");
        const size = this.getTileSize();
        const zoom = this._tileZoom;
        div.style.width = size.x;
        div.style.height = size.y;

        const p = new Point3D(coords.x, coords.y, zoom);

        const projected = project(coords.x, coords.y, zoom, 0, 0, TileSize);
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
      }
    });

    const mandelbrotLayer = new L.GridLayer.MandelbrotLayer({
      tileSize: TileSize, // in px
      bounds
    });
    const debugLayer = new L.GridLayer.DebugLayer({
      tileSize: TileSize,
      bounds
    });
    const workerLayer = new WorkerLayer(websocketClient, map.unproject.bind(map), this.props.group);
    const baseLayer = {
      "Mandelbrot Layer": mandelbrotLayer
    };
    const overlayLayers = {
      "Debug Layer": debugLayer,
      "Worker Layer": workerLayer
    };
    map.addLayer(mandelbrotLayer);
    map.addLayer(workerLayer);

    L.control.layers(baseLayer, overlayLayers).addTo(map);

    this.props.viewCenter.subscribe(pt => {
      if (!pt.equals(this.center)) {
        const p = complexToLeaflet(pt.x, pt.y, pt.z);
        map.setView([p.x, p.y], p.z);
        this.updateAllViews();
      }
    });
    // change URL params when region changes
    this.props.viewCenter.subscribe(center => setURLParams(center));

    map.addControl(
      L.control.zoomBox({
        modal: true,
        title: "Box area zoom"
      })
    );
    this.center = this.props.viewCenter.get();
    const p = complexToLeaflet(this.center.x, this.center.y, this.center.z);
    this.map.setView([p.x, p.y], p.z);
  }

  private updateAllViews() {
    // if the container size of the map has been changed, update the leaflet internal size to match
    this.map.invalidateSize();
    const center = this.map.getCenter();
    this.center = leafletToComplex(center.lat, center.lng, this.map.getZoom());
    this.props.viewCenter.set(this.center);
    this.newViewObservers.forEach(callback => callback(this.map));
  }
}
