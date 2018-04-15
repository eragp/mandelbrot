import 'leaflet/dist/leaflet.css';
import './index.css';
import L from 'leaflet';

// demo loading the london city map
let londonMap = L.map('map01').setView([51.505, -0.09], 13);
L.tileLayer('http://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
  attribution: 'OpenStreetMaps',
}).addTo(londonMap);

console.log('This is a test');
