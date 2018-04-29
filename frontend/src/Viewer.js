import React, { Component } from 'react';
// leaflet stuff
import 'leaflet/dist/leaflet.css';
import L from 'leaflet';
// custom stylesheet
import './Viewer.css';
import m from './img/mandelbrot.png';
import generate from './GeoJSON';

class Viewer extends Component {
  componentDidMount() {
    renderLeaflet();
  }

  render() {
    return <div id="viewer" />;
  }
}

const renderLeaflet = () => {
  let map = L.map('viewer', {
    crs: L.CRS.Simple,
    minZoom: 2
  });
  let bounds = [[0, 0], [300, 400]];
  L.imageOverlay(m, bounds).addTo(map);
  map.fitBounds(bounds);
  L.geoJSON(generate(), { style: styleUsageMap }).addTo(map);
};
const styleUsageMap = feature => {
  return {
    fillColor: getColor(feature.properties.time),
    weight: 1,
    opacity: 1,
    color: 'white',
    dashArray: '3',
    fillOpacity: 0.5
  };
};

const getColor = value => {
  return value > 1000
    ? '#800026'
    : value > 500
      ? '#BD0026'
      : value > 200
        ? '#E31A1C'
        : value > 100
          ? '#FC4E2A'
          : value > 50
            ? '#FD8D3C'
            : value > 20 ? '#FEB24C' : value > 10 ? '#FED976' : '#FFEDA0';
};

export default Viewer;
