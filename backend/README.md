# C++ Backend

## Quick Start

On Unix-like machines, make sure to have `rsync`, `ssh` and `python3` (3.5 or newer) installed.

Also make sure to have copied your ssh-key to the vm-schulz when using `-b` (build).

From this folder (`backend`) run
```bash
./run_himmuc.sh -h
```

For running from other hosts and more details, please refer to the documentation in `himmuc`.

### Development

To run the build pipeline a current version of `docker` needs to 
be installed on the host system

For testing the app on the local machine just run the `build` script `run_docker.sh` from this directory.

```bash
$ ./run_docker.sh
```

The first execution of the script will take some time as `docker` needs to download the base images and compile the libraries (~20 min). Successive runs are a lot faster due to caching.

After the script has been executed the compiled MPI program will be launched (with the settings from `runconfig`). After pressing `ctrl+c` an interactive bash is left open.

To detach from the container exit the current shell. You will be returned to the previous working directory on the local system

```bash
$ exit
```

A websocket connection to the hosted service inside the docker container can be established by connecting to `ws://localhost:9002`

### Deployment

To deploy the compiled `host` or `worker` binary copy it to the target system.

## Developer notes

The host uses three threads to manage all the communication. One thread listens for incoming requests on the websocket, performs the loadbalancing and stores the subregions in a shared data structure. The second thread does all the MPI communication with the workers. If a worker finishes its computation this thread will receive the result and store it in a shared data structure. The third thread will send the results to the frontend.

The workers will listen for incoming MPI and perform the computation of the fractal. During computation they listen for new requests. If there is a new request the running computation will be aborted.

### Adding new Balancers and Fractals

The class hierachies for both the fractals and the loadbalancers follow the strategy pattern. So to add a fractal just subclass `Fractal`, in order to add a balancer you will need to inherit from `Balancer`.

### Useful structs

The backend is intended to use three main structs for storing and passing information about the regions that are to be computed. Those are `Region`, `WorkerInfo` and `RegionData`. The important parts of those structs are included below.

```cpp
/**
* Used to describe a region of the fractal
*/
struct Region {

    /**
    * The coordinates for the top left corner of the region
    * in the complex plane.
    * Included in the region.
    */
    precision_t minReal, maxImaginary;

    /**
    * The coordinates for the bottom right corner of the region
    * in the complex plane.
    * Excluded from the region.
    */
    precision_t maxReal, minImaginary;

    /**
    * Pixel amount in respective dimension of this region.
    * Equivalent to resolution in x- and y- dimension.
    */
    unsigned int width, height;

    /**
    * Maximum n value for iteration in this region.
    */
    unsigned short int maxIteration;

    /**
    * Frontend specific information, identification/validation value, do not touch
    * With leaflet frontend equivalent to zoomfactor.
    * Used to decide whether the data is still needed.
    */
    int validation;

    /**
    * Value for which it is guaranteed that width and height are divisible by.
    * Same goes for hOffset and vOffset
    * Recursivley applies to all subregions
    */
    unsigned int guaranteedDivisor;

    /**
    * Horizontal and vertical offset respective to superregion.
    * Equivalent to xOffset/yOffset.
    * Should not become negative usually.
    */
    int hOffset, vOffset;

    /**
     * Value to determine, which fractal will be computed
     */
    enum fractal_type fractal;

    /**
     * Do not change.
     * Number of regions the corresponding big region got split into.
     */
    unsigned short int regionCount;
};
```

```cpp
/**
 * Information sent by Worker when done with computing data.
 * Contains constant amount of metadata.
 */
struct WorkerInfo {

    /**
     * The rank of the worker, that has computed the following data
     */
    int rank;

    /**
     * The time that computation has taken in nanoseconds
     */
    long long computationTime;

    /**
     * The region that was computed.
     */
    Region region;
    
};
```

```cpp
/**
 * Contains data of a certain region.
 * Usually assembled at the Host and used to transfer data to frontend
 */
struct RegionData {
    
    /**
     * Information about which worker computed the region, 
     * including the region itself, computation time and other metadata
     */
    WorkerInfo workerInfo;

    /**
     * The approximate time that MPI communication has taken in microseconds
     */
    unsigned long mpiCommunicationTime;

    /**
     * Raw computed data for the fractal in this region.
     * Size: region.width*region.height
     * Index: i (x,y) -> (region.width * y) + region.height
     */
    unsigned short int *data;
    int data_length;

};
```