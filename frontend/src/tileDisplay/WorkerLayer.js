import L from 'leaflet/dist/leaflet-src.js';
import {unproject} from './Project';

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


function toGeoJSON(regions){
  const featureCollection = {
    type: "FeatureCollection",
    features: []
  }

  for(const region of regions){
    featureCollection.features.push(
      {
        type: "Feature",
        geometry: {
          type: "Polygon",
          coordinates: [
            [
              // TODO correct coordinates (as arrays! lat, lon)
              unproject(region.minReal, region.maxImaginary, region.validation),
              unproject(region.maxReal, region.maxImaginary, region.validation),
              unproject(region.maxReal, region.minImaginary, region.validation),
              unproject(region.minReal, region.minImaginary, region.validation)
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

  constructor(wsclient){
    super(null, {
      style: style
    });
    this.wsclient = wsclient;
    wsclient.registerRegion(data => {
      const regions = toGeoJSON(data.regions);
      console.log(regions, this);
      this.clearLayers();
      this.addData(regions);
    });
  }

}