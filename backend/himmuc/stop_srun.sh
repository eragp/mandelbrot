#!/bin/bash

# Script to be run when the backend script did not stop gracefully
ssh $1@himmuc.caps.in.tum.de "scancel -u $1"