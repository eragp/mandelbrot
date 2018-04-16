import React, { Component } from 'react';
// leaflet stuff
import 'leaflet/dist/leaflet.css';
import L from 'leaflet';
// custom stylesheet
import './Viewer.css';
import m from './img/mandelbrot.png';

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
    minZoom: -2
  });
  let bounds = [[0, 0], [3000, 4000]];
  console.log(m);
  L.imageOverlay(m, bounds).addTo(map);
  map.fitBounds(bounds);
};

export default Viewer;
