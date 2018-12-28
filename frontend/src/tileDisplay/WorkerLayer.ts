import L from "leaflet/dist/leaflet-src.js";
import { Point, LatLng, GeoJSON } from "leaflet";
import { unproject } from "./Project";
import { TileSize } from "../Constants";
import WebSocketClient from "../connection/WSClient";
import { Feature, FeatureCollection } from "geojson";
import { RegionGroup } from "../misc/RegionGroup";
import { GroupObservable } from "../misc/Observable";

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
    const pixelCoords = L.point(tl.x * TileSize, -tl.y * TileSize);
    const latLng = pixelToLatLng(pixelCoords);
    return [latLng.lng, latLng.lat];
  };

  for (const region of regions) {
    featureCollection.features.push({
      type: "Feature",
      geometry: {
        type: "Polygon",
        coordinates: [region.bounds().map(p => toLatLngArray(p.x, p.y, region.validation))]
      },
      properties: {
        node: region.id,
        zoom: region.validation,
        isGroup: region.isGroup()
      }
    });
  }

  return featureCollection;
}
/**
 * Displays an overlay showing which node computed which region
 */
export default class WorkerLayer extends L.GeoJSON {
  private nodeLayers: Map<number, GeoJSON<any>>;
  private nodeGroups: Map<number, RegionGroup>;
  private currentGroup: RegionGroup[];
  private workerContext: GroupObservable;

  constructor(
    wsclient: WebSocketClient,
    pixelToLatLng: (p: Point) => LatLng,
    workerContext: GroupObservable
  ) {
    const style = (feature: Feature): {} => {
      let regionStyle = {
        weight: 1.5,
        opacity: 1,
        color: "white",
        fillColor: "white",
        dashArray: "3",
        fillOpacity: 0.3
      };
      if (
        feature.properties &&
        feature.properties.isGroup &&
        feature.properties.node === this.workerContext.get()
      ) {
        regionStyle = Object.assign(regionStyle, {
          fillColor: workerContext.getColor(feature.properties.node),
          fillOpacity: 0,
          weight: 10,
          dashArray: "0",
          color: "red"
        });
      } else {
        regionStyle = Object.assign(regionStyle, {
          fillOpacity: 0,
          weight: 1
          // dashArray: "0",
          // color: "red",
        });
      }
      return regionStyle;
    };

    const onEachFeature = (feature: Feature, layer: GeoJSON): void => {
      let node = 0;
      if (feature.properties !== null) {
        node = feature.properties.node;
      }
      layer.on({
        mouseover: () => {
          if (feature.properties !== null && feature.properties.isGroup) workerContext.set(node);
        },
        mouseout: () => {
          if (feature.properties !== null && feature.properties.isGroup)
            workerContext.set(undefined);
        }
      });
      this.nodeLayers.set(node, layer);
    };

    super(undefined, {
      style,
      onEachFeature
    });

    this.nodeLayers = new Map();
    this.nodeGroups = new Map();
    this.workerContext = workerContext;

    const onNewRegion = (group: RegionGroup[]) => {
      this.clearLayers();
      this.nodeGroups.clear();
      group.forEach(g => this.nodeGroups.set(g.id, g));
      this.currentGroup = group;

      const regions = toGeoJSON(group, pixelToLatLng);
      this.addData(regions);
    };
    wsclient.registerRegion(onNewRegion);

    workerContext.subscribe((worker: number | undefined) => {
      this.nodeLayers.forEach((layer: GeoJSON) => {
        this.resetStyle(layer);
      });
      if (worker) {
        const layer = this.nodeLayers.get(worker);
        if (layer) {
          layer.setStyle({
            fillOpacity: 0.7
          });
        }
        const group = this.nodeGroups.get(worker);
        if (group && group.getChildren() !== null) {
          onNewRegion(this.currentGroup);
          this.addData(toGeoJSON(group.getChildren() as RegionGroup[], pixelToLatLng));
        }
      }
    });
  }
}
