import L from 'leaflet/dist/leaflet-src.js';
import {unproject} from './Project';
import { tileSize } from './Constants';

/**
{
  "type": "FeatureCollection",
  "features": [{
      "type": "Feature",
      "geometry": {
          "type": "Polygon",
          "coordinates": [
                   [
                       [100.0, 0.0],
                       [101.0, 0.0],
                       [101.0, 1.0],
                       [100.0, 1.0],
                       [100.0, 0.0]
                   ]
                ]
      },
      "properties": {
          "prop0": "value0"
      }
  }, {
*/

/**
 * Colors for the workers
 * TODO replace with nice colorset (i.e. theme)
 */
const colorSet = [
  '#4661EE',
  '#EC5657',
  '#1BCDD1',
  '#8FAABB',
  '#B08BEB',
  '#3EA0DD',
  '#F5A52A',
  '#23BFAA',
  '#FAA586',
  '#EB8CC6',
];


/**
 * 
 * @param {Array} regions 
 * @param {Function} pixelToLatLng 
 */
function toGeoJSON(regions, pixelToLatLng){
  const featureCollection = {
    type: "FeatureCollection",
    features: []
  }

  const toLatLngArray = (real, imag, zoom) => {
    const tl = unproject(real, imag, zoom);
    const pixelCoords = L.point(tl.x * tileSize, -tl.y * tileSize);
    const latLng = pixelToLatLng(pixelCoords);
    return [latLng.lng, latLng.lat];
  };

  for(const region of regions){

    featureCollection.features.push(
      {
        type: "Feature",
        geometry: {
          type: "Polygon",
          coordinates: [
            [
              // TODO correct coordinates (as arrays! lat, lon)
              toLatLngArray(region.minReal, region.maxImag, region.validation),
              toLatLngArray(region.maxReal, region.maxImag, region.validation),
              toLatLngArray(region.maxReal, region.minImag, region.validation),
              toLatLngArray(region.minReal, region.minImag, region.validation)
            ]
          ]
        },
        properties: {
          node: region.nodeID,
          zoom: region.validation
        }
      }
    )
  }

  return featureCollection;
}

function style(feature) {
  return {
      fillColor: colorSet[feature.properties.nodeID],
      weight: 2,
      opacity: 1,
      color: 'white',
      dashArray: '3',
      fillOpacity: 0.7
  };
}

export default class WorkerLayer extends L.GeoJSON {

  constructor(wsclient, pixelToLatLng){
    super(null, {
      style: style
    });
    wsclient.registerRegion(data => {
      this.clearLayers();
      const regions = toGeoJSON(data.regions, pixelToLatLng);
      this.addData(regions);
    });
  }

}