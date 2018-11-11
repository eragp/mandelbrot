#!/bin/bash

DOCKER_IMAGE=rasaford/mandelbrot
IMAGE_VERSION=0.8

echo "Starting the Build Process"
## Docker ##
# check for new base image
docker pull $DOCKER_IMAGE:$IMAGE_VERSION
# remove old images
docker rmi $(docker images --filter "dangling=true" -q)
docker rm $(docker ps -aq -f "status=exited")
docker build -t $DOCKER_IMAGE:$IMAGE_VERSION .
# this needs to be the last command
# beacause it attaches the current shell to the running container
docker run --rm -it \
	--name mandelbrot_build \
	-p 8080:80 \
	-p 9002:9002 \
	$DOCKER_IMAGE:$IMAGE_VERSION
