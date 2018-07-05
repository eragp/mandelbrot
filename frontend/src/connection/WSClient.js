export default class {
  constructor(){
    /**
     * Callbacks for any methods interested in new region subdivisions or regionData (=result of one worker)
     */
    this.regionCallback = [];
    let regionCallback = this.regionCallback;
    this.workerCallback = [];
    let workerCallback = this.workerCallback;

    // Web Socket setup
    let url = 'ws://localhost:9002';
    let socket;
    // Necessary due to a backend bug
    {
      let s = new WebSocket(url);
      s.onclose(() => {
        socket = new WebSocket(url); //, 'mandelbrot');
      })
      s.onopen(()=> {
        s.close();
      })
    }
    // Buffer of requests to be sent when the socket connects
    const regionRequests = [];
    socket.onopen = () => {
      regionRequests.forEach(m => socket.send(m));
    };

    // Restart the socket connection on close (optional, as the frontend does not get a notification
    // that the connection failed on the first try)
    socket.onclose = () => {
      setTimeout(
        () => {
          socket = new WebSocket(url)
        }, 30000);
        // TODO maybe in more beautiful, less annoying
        //alert('Websocket connection failed, reconnecting in 30s')
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
          });
          break;
        default:
      }
    };

    this.socket = socket;
  }

  /**
   * Registers a callback to call when the region subdivision is returned
   */
  registerRegion(fun){
    this.registerCallback(this.regionCallback, fun);
  }

  /**
   * Registers a callback to call when the region data is returned
   */
  registerWorker(fun){
    this.registerCallback(this.workerCallback, fun);
  }

  /**
   * Registers an observer to a list
   */
  registerCallback(list, fun){
    let promise;
    const render = data => {
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
  }

  close(){
    console.log('closing the WS connection');
    this.socket.close();
  }

  sendRequest(request){
    let message = JSON.stringify(request);
    if (this.socket.readyState === this.socket.OPEN) {
      this.socket.send(message);
    } else {
      this.regionRequests.push(message);
    }
  }

}
