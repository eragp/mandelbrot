import L from "leaflet/dist/leaflet-src.js";
import { unproject } from "./Project";
import { tileSize } from "./Constants";

/**
 *
 * @param {Array} regions
 * @param {Function} pixelToLatLng
 */
function toGeoJSON(regions, pixelToLatLng) {
  const featureCollection = {
    type: "FeatureCollection",
    features: []
  };

  const toLatLngArray = (real, imag, zoom) => {
    const tl = unproject(real, imag, zoom);
    const pixelCoords = L.point(tl.x * tileSize, -tl.y * tileSize);
    const latLng = pixelToLatLng(pixelCoords);
    return [latLng.lng, latLng.lat];
  };

  for (const region of regions) {
    featureCollection.features.push({
      type: "Feature",
      geometry: {
        type: "Polygon",
        coordinates: [
          [
            // TODO correct coordinates (as arrays! lat, lon)
            toLatLngArray(region.minReal, region.maxImag, region.validation),
            toLatLngArray(region.maxReal, region.maxImag, region.validation),
            toLatLngArray(region.maxReal, region.minImag, region.validation),
            toLatLngArray(region.minReal, region.minImag, region.validation),
            toLatLngArray(region.minReal, region.maxImag, region.validation)
          ]
        ]
      },
      properties: {
        node: region.nodeID,
        zoom: region.validation
      }
    });
  }

  return featureCollection;
}

export default class WorkerLayer extends L.GeoJSON {
  constructor(wsclient, pixelToLatLng, workerContext) {
    const style = feature => {
      return {
        weight: 1.5,
        opacity: 1,
        color: "white",
        fillColor: workerContext.getWorkerColor(feature.properties.node),
        dashArray: "3",
        fillOpacity: 0.3
      };
    };

    const onEachFeature = (feature, layer) => {
      layer.on({
        mouseover: () => workerContext.setActiveWorker(feature.properties.node),
        mouseout: () => workerContext.setActiveWorker(undefined)
      });
      this.nodeLayers.set(feature.properties.node, layer);
    };

    super(null, {
      style: style,
      onEachFeature: onEachFeature
    });

    this.nodeLayers = new Map();

    wsclient.registerRegion(data => {
      this.clearLayers();
      const regions = toGeoJSON(data.regions, pixelToLatLng);
      this.addData(regions);
    });

    workerContext.subscribe(worker => {
      this.nodeLayers.forEach(layer => {
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
