# the gcc dockerfile uses the buildpack-deps package and is based on debian:stretch
FROM gcc:7
WORKDIR /usr/src/mandelbrot-backend
# install required dependencies
RUN update-alternatives --install /usr/bin/gfortran gfortran /usr/local/bin/gfortran 999 \
    && apt-get update && apt-get install -y \
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
    libcrypto++-dev \
    libpthread-workqueue-dev \
    # mpich dependencies
    mpich \
    gfortran \
    libtool \
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

# based on Dockerfile from: wget https://github.com/sourceryinstitute/mpich-docker-image/blob/master/Dockerfile
# install mpich 3.2.1
ENV MPI_HOME /opt/mpich/3.2.1/gnu/6.2.1
RUN wget -q -O - https://www.mpich.org/static/downloads/3.2.1/mpich-3.2.1.tar.gz | tar xzv -C . \
    && mkdir mpich-3.2.1/build \
    && cd mpich-3.2.1/build \
    && ../configure --prefix="$MPI_HOME" \
    && make -j "$(nproc)" \
    && make install \
    && export PATH="$MPI_HOME/bin:$PATH" \
    && mpirun --version \
    && mpif90 -show \
    && mpif90 --version \
    && make clean \
    && make distclean \
    && cd ../../ \
    && rm -rf mpich-3.2.1 \
    # run some tests on the installation to verify it
    # c test
    && wget https://raw.githubusercontent.com/wesleykendall/mpitutorial/master/tutorials/mpi-hello-world/code/mpi_hello_world.c \
    && mpicc mpi_hello_world.c -o hello-c \
    && mpirun -np 4 ./hello-c \
    # fortran 90 test
    && wget https://people.sc.fsu.edu/~jburkardt/f_src/hello_mpi/hello_mpi.f90 \
    && mpif90 hello_mpi.f90 -o HELLO-f90 \
    && mpirun -np 4 ./HELLO-f90 \
    && cd ../ \
    && rm -rf mpi-tests \
    && rm -rf /var/lib/apt/lists/* /var/log/* /tmp/* \
    && echo "export PATH=\"$MPI_HOME/bin:\$PATH\"" >> /etc/bash.bashrc

# install websocketpp v0.7.0
RUN git clone --branch 0.7.0 https://github.com/zaphoyd/websocketpp.git websocketpp \
    && cd websocketpp \
    && mkdir build \
    && cd build \
    && cmake .. \
    && make \
    && make install \
    && cd ../..
