#!/bin/bash

echo "Starting the Build Process"
## Docker ##
# remove old images
docker rmi $(docker images --filter "dangling=true" -q)
docker rm $(docker ps -aq -f "status=exited")
docker build -t mandelbrot:0.1 .
# this needs to be the last command
# beacause it attaches the current shell to the running container
docker run --rm -it \
    --name mandelbrot_build \
	-p 4000:80 mandelbrot:0.1 \
    /bin/bash
