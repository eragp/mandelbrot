import 'leaflet/dist/leaflet.css';
import L from 'leaflet';


// const renderTile = async () => {
//   console.log('Requesting: ' + url);
//   try {
//     let response = await fetch(url, { method: 'GET', mode: 'cors' });
//     let json = await response.json();
//     console.log(json);
//     for (let y = 0; y < size; y++) {
//       for (let x = 0; x < size; x++) {
//         let n = json[x + y * size];
//         let r = (n * 10) % 256;
//         let g = (n * 20) % 256;
//         let b = (n * 40) % 256;
//         ctx.fillStyle = 'rgba(' + r + ',' + g + ',' + b + ', 255)';
//         ctx.fillRect(x, y, x, y);
//       }
//     }
//   } catch (exception) {
//     console.error(exception);
//   }
// };
// renderTile();
//drawing the shape of the point
//adding gradient
