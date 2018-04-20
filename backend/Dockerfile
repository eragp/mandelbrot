# the gcc dockerfile uses the buildpack-deps package and is based on debian:stretch
FROM gcc:7
WORKDIR /usr/src/mandelbrot-backend
# install required dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    libccprest-dev \
    zliblg-dev \
    libboost-all-dev \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# changes that depend on src/ only after this point
COPY . /usr/src/mandelbrot-backend
RUN cd build && \
    cmake .. && \
    make && \
    mv *.out .. &&  \
    cd ..
CMD ["/bin/bash"]
