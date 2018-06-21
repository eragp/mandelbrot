/*
 * this map stores callbacks to render all the tiles requested for leaflet
 */
const callbacks = new Map();
// Web Socket setup
const url = 'ws://localhost:80/mandelbrot';
const socket = new WebSocket(url, 'protocolOne');
socket.onmessage = onWSMessage;

function onWSMessage(event) {
  let msg = JSON.parse(event);
  switch (msg.type) {
    case 'tile':
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
  callbacks.set(coordsToString(coords), render);
  return promise;
};
/**
 * renders all available tiles
 */
export const render = () => {
  let data = 'this is a test';
  console.log('rendering requested tiles');
  for (let renderFunc of callbacks.values()) {
    renderFunc(data);
  }
  callbacks.clear();
};

export const close = () => {
  console.log('closing the connection');
  socket.close();
  callbacks.clear();
};

function coordsToString(coords) {
  return [coords.x, coords.y, coords.z].join(', ');
}
