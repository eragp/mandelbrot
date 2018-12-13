FROM rasaford/mandelbrot:1.1

ENV HOME /home/username
RUN mkdir -p ${HOME}/eragp-mandelbrot/backend
# changes that depend on src/ only after this point
COPY . ${HOME}/eragp-mandelbrot/backend
# shared library path needs to be set
ENV LD_LIBRARY_PATH /usr/local/lib

WORKDIR ${HOME}/eragp-mandelbrot/backend
RUN cd build \
    && cmake .. \
    && make 

# Runs bash to start a bash that, on startup
# runs mpiexec and is afterwards left open for interaction
# Source: https://serverfault.com/questions/368054/run-an-interactive-bash-subshell-with-initial-commands-without-returning-to-the/586272#586272
# Necessary for the hydra process manager to work correctly
RUN /bin/bash --init-file <(echo 'mpiexec -configfile $HOME/eragp-mandelbrot/backend/runconfig')
