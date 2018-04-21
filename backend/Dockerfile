# the gcc dockerfile uses the buildpack-deps package and is based on debian:stretch
FROM gcc:7
WORKDIR /usr/src/mandelbrot-backend
# install required dependencies
RUN apt-get update && apt-get install -y \
    # build tools
    cmake \
    make \
    g++ \
    git \
    # CLI tools to make life easier while in the container
    vim \
    nano \
    # libs
    libcpprest-dev \
    zlib1g-dev \
    libboost-all-dev \
    libssl-dev \
    # clean after apt-get install
    && rm -rf /var/lib/apt/lists/*
# build cpprestsdk from source
RUN git clone https://github.com/Microsoft/cpprestsdk.git casablanca \
    && cd casablanca/Release \
    && mkdir build.debug \
    && cd build.debug \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make \
    && make install 

# changes that depend on src/ only after this point
COPY . /usr/src/mandelbrot-backend
RUN cd build \
    && cmake .. \
    && make 

CMD ["/bin/bash"]
