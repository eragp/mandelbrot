FROM rasaford/mandelbrot:latest
# changes that depend on src/ only after this point
COPY . /usr/src/mandelbrot-backend
# shared library path needs to be set
ENV LD_LIBRARY_PATH /usr/local/lib

RUN cd build \
    && cmake .. \
    && make 

CMD ["/usr/src/mandelbrot-backend/build/mandelbrot"]