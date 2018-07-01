import Point from '../misc/Point';

/*
 * this map stores callbacks to render all the tiles requested for leaflet.
 */
const callbacks = new Map();
/**
 * Callbacks for any methods interested in new region subdivisions or regionData (=result of one worker)
 */
const regionCallback = new Array();
const workerCallback = new Array();

// Web Socket setup
const url = 'ws://localhost:9002';
export const socket = new WebSocket(url); //, 'mandelbrot');
// Buffer of requests to be sent when the socket connects
const regionRequests = [];
socket.onopen = () => {
  regionRequests.forEach(m => socket.send(m));
};

socket.onmessage = function(event) {
  let msg = JSON.parse(event.data);
  switch (msg.type) {
    case 'tile':
      {
        // Notify tile listeners
        console.log(msg);
        let coords = coordsToString(msg.tile.x, -msg.tile.y, msg.tile.zoom);
        let cb = callbacks.get(coords);
        if (cb != null) {
          cb(msg.data);
          // callbacks.delete(coords);
        } else {
          console.log('request not found for tile: ' + coords);
        }
        // Notify regionData/worker observers
        workerCallback.forEach(callback => {
          callback(msg);
        });
      }
      break;
    case 'regions':
      {
        // Notify region subdivision listeners
        console.log(msg);
        regionCallback.forEach(callback => {
          callback(msg);
        })
      }
      break;
    default:
      console.log(msg);
  }
};

/**
 *  Registers the tile at coords to be drawn as soon as data is available.
 * @param {*} point coordinates on the tile to be registerd
 * @param {*} draw function expecting data that draws the tile @coords
 */
export const register = (point, draw) => {
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
  let coords = coordsToString(point.x, point.y, point.z);
  callbacks.set(coords, render);
  return promise;
};

/**
 * Registers a callback to call when the region subdivision is returned
 */
export const registerRegion = (fun) => {
  registerCallback(regionCallback, fun);
}

/**
 * Registers a callback to call when the region subdivision is returned
 */
export const registerWorker = (fun) => {
  registerCallback(workerCallback, fun);
};

/**
 * Registers an observer to a list
 */
const registerCallback = (list, fun) => {
  let promise;
  const render = (data) => {
    promise = new Promise((resolve, error) => {
      try {
        resolve(fun(data));
      } catch (err) {
        error(err);
      }
    });
  };
  list.push(render);
  return promise;
};

export const close = () => {
  console.log('closing the WS connection');
  socket.close();
  callbacks.clear();
};

export const sendRequest = request => {
  let message = JSON.stringify(request);
  if (socket.readyState === socket.OPEN) {
    socket.send(message);
  } else {
    regionRequests.push(message);
  }
};

function coordsToString(x, y, z) {
  return [x, y, z].join(', ');
}

export default socket;
