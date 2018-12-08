import L from "leaflet/dist/leaflet-src.js";
import { Point, LatLng, GeoJSON } from "leaflet";
import { unproject } from "./Project";
import { tileSize } from "./Constants";
import WebSocketClient from "../connection/WSClient";
import WorkerContext from "../misc/WorkerContext";
import { Feature, FeatureCollection } from "geojson";
import { RegionGroup } from "../connection/RegionGroup";

/**
 *
 * @param regions
 * @param pixelToLatLng
 */
function toGeoJSON(regions: RegionGroup[], pixelToLatLng: (a: Point) => LatLng): FeatureCollection {
  const featureCollection: FeatureCollection = {
    type: "FeatureCollection",
    features: Array<Feature>()
  };

  const toLatLngArray = (real: number, imag: number, zoom: number): number[] => {
    const tl = unproject(real, imag, zoom);
    const pixelCoords = L.point(tl.x * tileSize, -tl.y * tileSize);
    const latLng = pixelToLatLng(pixelCoords);
    return [latLng.lng, latLng.lat];
  };

  for (const region of regions) {
    let bounds = region.bounds().map(p => toLatLngArray(p.x, p.y, region.validation));
    featureCollection.features.push({
      type: "Feature",
      geometry: {
        type: "Polygon",
        coordinates: [bounds]
      },
      properties: {
        node: region.rank,
        zoom: region.validation
      }
    });
  }

  return featureCollection;
}
/**
 * Displays an overlay showing which node computed which region
 */
export default class WorkerLayer extends L.GeoJSON {
  constructor(
    wsclient: WebSocketClient,
    pixelToLatLng: (p: Point) => LatLng,
    workerContext: WorkerContext
  ) {
    const style = (feature: Feature): {} => {
      const ret = {
        weight: 1.5,
        opacity: 1,
        color: "white",
        fillColor: "white",
        dashArray: "3",
        fillOpacity: 0.3
      };
      if (feature.properties !== null) {
        ret.fillColor = workerContext.getWorkerColor(feature.properties.node);
      }
      return ret;
    };

    const onEachFeature = (feature: Feature, layer: GeoJSON): void => {
      let node = 0;
      if (feature.properties !== null) {
        node = feature.properties.node;
      }
      layer.on({
        mouseover: () => workerContext.setActiveWorker(node),
        mouseout: () => workerContext.setActiveWorker(undefined)
      });
      this.nodeLayers.set(node, layer);
    };

    super(undefined, {
      style,
      onEachFeature
    });

    this.nodeLayers = new Map();

    wsclient.registerRegion((data: RegionGroup[]) => {
      this.clearLayers();
      const regions = toGeoJSON(data, pixelToLatLng);
      this.addData(regions);
    });

    workerContext.subscribe((worker: number | undefined) => {
      this.nodeLayers.forEach((layer: GeoJSON) => {
        this.resetStyle(layer);
      });
      if (worker !== undefined) {
        const layer = this.nodeLayers.get(worker);
        if (layer) {
          layer.setStyle({
            fillOpacity: 0.7
          });
        }
      }
    });
  }
}
