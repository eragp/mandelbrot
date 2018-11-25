#!/bin/bash

DOCKER_REMOTE_IMAGE_NAME="rasaford/mandelbrot"
IMAGE_VERSION="1.1"
LOCAL_IMAGE_NAME="mandelbrot_build"
LOCAL_VERSION="1.1"

echo "Starting the Build Process"
## Docker ##
# check for new base image
docker pull $DOCKER_REMOTE_IMAGE_NAME:$IMAGE_VERSION
# remove old images
docker rmi $(docker images --filter "dangling=true" -q)
docker rm $(docker ps -aq -f "status=exited")
docker build -t $LOCAL_IMAGE_NAME:$LOCAL_VERSION .
# this needs to be the last command
# beacause it attaches the current shell to the running container
docker run --rm -it \
	--name mandelbrot_build \
	-p 8080:80 \
	-p 9002:9002 \
	$LOCAL_IMAGE_NAME:$LOCAL_VERSION
