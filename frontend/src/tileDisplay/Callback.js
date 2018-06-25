import Point from "../misc/Point";

/*
 * this map stores callbacks to render all the tiles requested for leaflet.
 */
const callbacks = new Map();
/**
 * If callback is created after the data, the map stores data as well that
 * is passed directly to the tile render function
 */
const data = new Map();
// Web Socket setup
const url = 'ws://localhost:9002';
export const socket = new WebSocket(url); //, 'mandelbrot');
socket.onmessage = function (event) {
  let msg = JSON.parse(event.data);
  switch (msg.type) {
    case 'tile':
      console.log(msg);
      let coords = new Point(msg.tile.x, msg.tile.y, msg.tile.zoom)
      let cb = callbacks.get(coordsToString(coords));
      // console.log(coordsToString(coords));
      if (cb != null) {
        cb(msg.data);
      } else {
        data.set(coordsToString(coords), msg.data);
      }
      break;
    case 'regions':
      // TODO
    default:
      console.log(msg);
      break;
  }
}

/**
 *  Registers the tile at coords to be drawn as soon as data is available.
 * @param {*} coords coordinates on the tile to be registerd
 * @param {*} draw function expecting data that draws the tile @coords
 */
export const register = (coords, draw) => {
  let promise;
  const render = data => {
    promise = new Promise((resolve, error) => {
      try {
        draw(data);
        resolve();
      } catch (err) {
        error(err);
      }
    });
  };
  let dt = data.get(coordsToString(coords));
  if (dt != null) {
    render(dt);
  } else {
    callbacks.set(coordsToString(coords), render);
  }
  // console.log(coordsToString(coords));
  return promise;
};

export const close = () => {
  console.log('closing the connection');
  socket.close();
  callbacks.clear();
};

function coordsToString(coords) {
  return [coords.x, coords.y, coords.z].join(', ');
}

export default socket;