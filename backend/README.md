# c++ Backend

## Environment

> IMPORTANT: When using strings in cpprestsdk prefix them with the `U()` Makro:
> ```cpp
>listener(U("http://localhost/restdemo"))
> ```

## Quick Start

To run the build pipeline a current version of `docker` needs to 
be installed on the host system

### Development

For testing the app on the local machine just run the `build` script `backend/run.sh`.

```bash
$ ./run.sh
```

The first execution of the script will take some time as `docker` needs to download the base images and compile cpprest (~20 min). Successive runs are a lot faster due to caching .
> TODO: Upload the cached image to docker hub to avoid recompilation of libs

After the script has been executed the compiled MPI program will be launched (with correct settings, hopefully). After pressing ctrl+c an interactive bash is left open. If you want to reastart the binary run

```bash
$ mpiexec -f hosts ./build/mandelbrot
```

Parameter `-np {processes}` can be added to specify the number of parallel processes that should be started. Edit the file `hosts` to [add machines with ssh access or to specify the process slots on the local machine](https://wiki.mpich.org/mpich/index.php/Using_the_Hydra_Process_Manager).

To detach from the container exit the current shell. You will be returned to the previous working directory on the local system

```bash
$ exit
```

A connection to the hosted service inside the docker container can be established by connecting to `http://localhost:8080/mandelbrot?x=0&y=1&z=2&size=256` (i.e. via your browser, replace 0/1/2 with x/y/zoom coordinates).

### Deployment

To deploy the compiled `mandelbrot` binary copy it to the target system.

## Developer notes

>> TODO Update this section
Http requests are handled by the code in the new actors/Host.cpp. It starts a listener on the url specified above. Incoming requests are stored in an internal request dictionary and a computation command (tag 1) is issued to the next available core (managed through a queue).

Parallely the host listens to incoming messages (on tag 2), converts the received data to a JSON array and answers the corresponding request stored in the request dictionary. For finding the correct request, the dictionary identifiers are based on the coordinates of the tile to compute.


### Useful structs

The backend is intended to use three main structs for storing and passing information about the regions that are to be computed. Those are `Region`, `WorkerInfo` and `RegionData`. The important parts of those structs are included below.

```cpp
/**
 * The new and up-to-date Region struct for view and subdivided regions
 */
struct Region {

    /**
     * The coordinates for the top left corner of the region
     * in the complex plane. 
     * Included in the region.
     */
    long double minReal, maxImaginary;

    /**
     * The coordinates for the bottom right corner of the region
     * in the complex plane.
     * Excluded from the region.
     */
    long double maxReal, minImaginary;

    /**
     * Pixel amount in respective dimension of this region.
     * Equivalent to resolution in x- and y- dimension.
     */
    unsigned int width, height;

    /**
     * Maximum n value for iteration in this region.
     */
    unsigned int maxIteration;

    /**
     * Frontend specific information, identification/validation value, do not touch
     * With leaflet frontend equivalent to zoomfactor.
     */
    int validation;

    /**
     * Value for which it is guaranteed that width and height are divisible by.
     * Same goes for hOffset and vOffset
     * Recursivley applies to all subregions
     */
    unsigned int guaranteedDivisor; 

    /**
     * Horizonatl and vertical offset respective to superregion.
     * Equivalent to xOffset/yOffset.
     * Should not become negative usually.
     */
    int hOffset, vOffset;

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
struct RegionData{

    /**
     * Information about which region was computed.
     */
    Region region;

    /**
     * Information about which worker computed the region and other metadata
     */
    WorkerInfo workerInfo;

    /**
     * Raw computed data for the fractal in this region.
     * Size: region.width*region.height
     * Index: i (x,y) -> (region.width * y) + region.height
     */
    int data[];

};
```