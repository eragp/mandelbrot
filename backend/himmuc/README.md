# Set Up on Raspberry Pi

## Installation

If you want to build the executables on your own, procede. If you want to use the pre-compiled executables (for arch debian stretch raspis) jump directly to [Running](#running)

First, create a directory with write access. In this tutorial `~/.eragp-mandelbrot` will be used. Replace any occurrence of this path to choose your own path. Also make sure to change the path in backend/CMakeLists line 6 (include_directories) and line 19 (set_target_properties).

```bash
mkdir ~/.eragp-mandelbrot
```

1. Install Boost 

    (https://www.boost.org/doc/libs/1_68_0/more/getting_started/unix-variants.html)

    Only `boost_system` has to be built. The following also builds `boost_thread` and `boost_random` as they might be useful for building websocketpp.

    ```bash
    cd ~/.eragp-mandelbrot
    mkdir package
    cd package
    wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.bz2
    tar --bzip2 -xf boost_1_67_0.tar.bz2
    cd boost_1_67_0
    ./bootstrap.sh --prefix="$HOME/.eragp-mandelbrot/local/" --with-libraries=system,thread,random
    ./b2 install
    cd ..
    ```

2. Install websocketpp

    1. Install OpenSSL (optional, not tested)

    2. zlib is installed on the himmuc

    ```bash
    mkdir ~/.eragp-mandelbrot/install
    cd ~/.eragp-mandelbrot/install
    git clone --branch 0.7.0 https://github.com/zaphoyd/websocketpp.git websocketpp --depth 1
    ```

3. Install rapidjson

    ```
    mkdir ~/.eragp-mandelbrot/install
    cd ~/.eragp-mandelbrot/install
    git clone https://github.com/Tencent/rapidjson/
    ```

4. Activate MPI module

    ```bash
    module load mpi
    ```


5. Build mandelbrot

    CMakeLists.txt was changed such that all libraries above are included explicitely. Make sure that this is also true for your installation.

    Then clone the mandelbrot project where-ever you prefer (often `~/git`) and
    build the tools.
    ```bash
    git clone https://gitlab.lrz.de/lrr-tum/students/eragp-mandelbrot
    cd eragp_mandelbrot/backend

    mkdir build
    cd build
    cmake ..
    make
    ```
## Running

First log on to the himmuc. [More Information](http://www.caps.in.tum.de/hw/himmuc/quick-start/)

6. SSH to vmschulz (himmuc.caps.in.tum.de)

   ```bash
   ssh <login>@himmuc.caps.in.tum.de
   ```

7. Run the executables

   ```bash
   git clone https://gitlab.lrz.de/lrr-tum/students/eragp-mandelbrot # if not already done
   cd eragp-mandelbrot/backend/himmuc
   
   srun -n<number of nodes+1> -l --multi-prog run.conf
   ssh -L 0.0.0.0:9002:localhost:9002 -fN -M -S .tunnel.ssh rpi<lowest alloced number>
   ```

8. How to stop

  ```bash
  ssh -S .tunnel.ssh -O exit rpi<lowest alloced number>
  ```
