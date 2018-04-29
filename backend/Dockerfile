FROM rasaford/mandelbrot:latest
# changes that depend on src/ only after this point
COPY . /usr/src/mandelbrot-backend
# shared library path needs to be set
ENV LD_LIBRARY_PATH /usr/local/lib

RUN cd build \
    && cmake .. \
    && make 

# Runs bash to start a bash that, on startup
# runs mpiexec and is afterwards left open for interaction
CMD /bin/bash -c "/bin/bash --init-file <(echo 'mpiexec -f /usr/src/mandelbrot-backend/hosts /usr/src/mandelbrot-backend/build/mandelbrot')"
