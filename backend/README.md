# c++ Backend

## Installing

To run the build pipeline a current version of `docker` needs to 
be installed on the host system

## Development

For testing the app on the local machine just run the `build` script `backend/build.sh`.

```bash
$ ./build.sh
```

After the script has been executed the current shell will be connected to the container.
To detach from the container exit the current shell. You will be returned to the previous working directory on the local system

```bash
$ exit
```

## Deployment

To deploy the compiled `mandelbrot` binary copy it to the target system.
