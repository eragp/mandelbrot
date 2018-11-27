# How to install on RaspBerryPi

1. Install Boost 

    (https://www.boost.org/doc/libs/1_68_0/more/getting_started/unix-variants.html)

    Try: Only build boost.system as it should be the only boost library required so far
    Catch: Websocketpp need boost
    - system
    - thread

    ```bash
    wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.bz2
    tar --bzip2 -xf boost_1_67_0.tar.bz2
    ./bootstrap.sh --prefix="~/try_install/local/" --with-libraries=system,thread,random
    ./bz2 install
    ``

3. Activate MPI module

    ```bash
    module load mpi
    ```

4. Install websocketpp

    1. Install OpenSSL (likely optional, not tested)
        ```bash
        git clone https://github.com/openssl/openssl/ --depth 1
        ./config --prefix=~/try_install/openssl --openssldir=~/try_install/local/ssl
        make
        make install
        make test
        ```

    2. zlib is installed

    ```bash
    git clone --branch 0.7.0 https://github.com/zaphoyd/websocketpp.git websocketpp --depth 1
    ```

6. Install rapidjson

    ```
    git clone https://github.com/Tencent/rapidjson/
    ```

    Add rapidjson/include directory to include path of project

7. Make mandelbrot

    Change CMakeLists.txt such that all libraries above are included explicitely. Then 
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```


8. Now create a directory build (i.e. inside backend) and run
    ```bash
    cmake ..
    make 
    ```