/**
 * Callbacks for any methods interested in new region subdivisions or regionData (=result of one worker)
 */
const regionCallback = [];
const workerCallback = [];

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
  console.log(msg);
  switch (msg.type) {
    case 'regionData':
      // Notify regionData/worker observers
      workerCallback.forEach(callback => {
        callback(msg);
      });
      break;
    case 'regions':
      // Notify region subdivision listeners
      regionCallback.forEach(callback => {
        callback(msg);
      })
      break;
    default:
      
  }
};

/**
 * Registers a callback to call when the region subdivision is returned
 */
export const registerRegion = (fun) => {
  registerCallback(regionCallback, fun);
};

/**
 * Registers a callback to call when the region data is returned
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
};

export const sendRequest = request => {
  let message = JSON.stringify(request);
  if (socket.readyState === socket.OPEN) {
    socket.send(message);
  } else {
    regionRequests.push(message);
  }
};

export default socket;
