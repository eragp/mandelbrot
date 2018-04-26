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

A connection to the hosted service inside the docker container can be established by connecting to `http://localhost:8080/mandelbrot` (i.e. via your browser).

### Deployment

To deploy the compiled `mandelbrot` binary copy it to the target system.
