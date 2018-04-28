import React, { Component } from 'react';
// leaflet stuff
import 'leaflet/dist/leaflet.css';
import L from 'leaflet';
// custom stylesheet
import './Viewer.css';
import m from './img/mandelbrot.png';
import generate from './GeoJSON';
import $ from 'jquery';

class Viewer extends Component {
  componentDidMount() {
    renderLeaflet();
  }

  render() {
    return <div id="viewer" />;
  }
}

L.GridLayer.MandelBrotLayer = L.GridLayer.extend({

      createTile: function(coords, done){
          console.log("Called!" + coords);
          var error;
          // create a <canvas> element for drawing
          var tile = L.DomUtil.create('canvas', 'leaflet-tile');
          // setup tile width and height according to the options
          var size = this.getTileSize(); // default 256
          tile.width = size.x;
          tile.height = size.y;
          // Request image data for coords.x, coords.y and coords.z
          // Could be implemented through websocket
          // Request tiles from the backend
          // z isn't yet supported by the backend
          // r is completely optional
          // Documentation: http://leafletjs.com/reference-1.3.0.html#tilelayer
          var ctx = tile.getContext("2d");
          // draw something asynchronously and pass the tile to the done() callback
          $.get('http://localhost:8080/mandelbrot?x={x}&y={y}', {
            x: coords.x,
            y: coords.y
          }, function(data){
            console.log('called too!');
            var result = JSON.parse(data);
            for(var y = 0; y < size; y++){
             for(var x = 0; x < size; x++){
              var n = result[x+y*256]; 
              var r = n * 10 % 256;
              var g = n * 20 % 256;
              var b = n * 40 % 256;
              
              ctx.fillStyle = "rgba("+r+","+g+","+b+", 255)";
              ctx.fillRect(x,y,x,y);
             }
            }
            
          }, "json")
          .fail(function(jq, textStatus, errorThrown){
            console.log(textStatus +": " + errorThrown);
            error = textStatus;
          })
          .always(function(){
            done(error, tile);
          });


          return tile;
      }
});
L.gridLayer.mandelBrotLayer = function() {
    return new L.GridLayer.MandelBrotLayer();
}


const renderLeaflet = () => {
  let map = L.map('viewer', {
    crs: L.CRS.Simple,
    minZoom: 2
  });
  let imageSize = 2048; // sync with backend => maybe even through websocket at beginnig?

  let bounds = [[0, 0], [imageSize, imageSize]];
  //L.imageOverlay(m, bounds).addTo(map);
  
  
  L.gridLayer.mandelBrotLayer().addTo(map);
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
