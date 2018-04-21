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

After the script has been executed the current shell will be connected to the container.
Run the compiled application with:

```bash
$ ./mandelbrot
```

To detach from the container exit the current shell. You will be returned to the previous working directory on the local system

```bash
$ exit
```

### Deployment

To deploy the compiled `mandelbrot` binary copy it to the target system.
