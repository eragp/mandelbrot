# Set Up on Raspberry Pi

## Quick start

If you just want to get this running up and delivering, make sure you have installed
- `rsync`
- `ssh`
- `python3` (version 3.5 or newer)

and run (from the backend folder, one directory above this README)

```bash
python3 himmuc/start_himmuc.py --help
```

Also make sure to have copied your ssh-key to the vm-schulz when using `-b` (build).

Example output:
```
$ ./run_himmuc.sh muendler 10 9
Uploading backend...  sending incremental file list
backend/himmuc/start_backend.py
          3,897 100%    3.05MB/s    0:00:00 (xfr#1, to-chk=35/62)
done
Start mandelbrot with 1 host and 9 workers on 9 nodes... started mandelbrot
Search host node... srun: error: Could not find executable worker
odr00 found
Establish port 9002 forwarding to host node odr00:9002 ... established
System running. Websocket connection to backend is now available at
        ws://himmuc.caps.in.tum.de:9002
Press enter (in doubt, twice) to stop Warning: Permanently added the ED25519 host key for IP address '10.42.0.54' to the list of known hosts.


Stopping port forwarding... stopped (-9)
Stopping mandelbrot host and workers... stopped (-9)
```

## Installation

If you want to build the executables on your own, procede. If you want to use the pre-compiled executables (for arch debian stretch raspis) jump directly to [Running](#running)

First, create a directory with write access. In this tutorial `~/.eragp-mandelbrot` will be used. Replace any occurrence of this path to choose your own path. Also make sure to change the path in backend/CMakeLists line 6 (include_directories) and line 19 (set_target_properties).

```bash
mkdir ~/.eragp-mandelbrot
```

1. Install Boost 

    (https://www.boost.org/doc/libs/1_68_0/more/getting_started/unix-variants.html)

    Only `boost_system` has to be built.

    ```bash
    cd ~/.eragp-mandelbrot
    mkdir install
    cd install
    wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.bz2
    tar --bzip2 -xf boost_1_67_0.tar.bz2
    cd boost_1_67_0
    ./bootstrap.sh --prefix="$HOME/.eragp-mandelbrot/local/" --with-libraries=system
    ./b2 install
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
    cd eragp-mandelbrot/backend
    
    git checkout raspberry_pi_die_erste

    mkdir build
    cd build
    cmake ..
    make
    ```
## Running

### Using the automagically working python script

It is possible to set up the project in such a way that the execution on the himmuc
is handled automatically by the provided script `start_himmuc.py`.

Prerequisites:
 - `python3`
 - `ssh`
 - `rsync`

> Note: If `ssh` is not available on your device, clone the project on the vmschulz. Then login to the vmschulz and run
> ```bash
> python3 ~/eragp-mandelbrot/backend/himmuc/start_backend.py
> ```
> This is essentially what the script `start_himmuc.py` does.
> Also note that the script is not working directory sensitive, yet it is important that the git repository is cloned into the home directory

**Set up on local device**

Clone the repository.

**Running automagically**

Any time you want to run the backend on the himmuc, execute
`start_himmuc.py`, which is located in this directory from the backend folder (one directory up).

```bash
python3 himmuc/start_himmuc.py --help
```

### Running manually

6. SSH to vmschulz (himmuc.caps.in.tum.de)
   First log on to the himmuc. [More Information](http://www.caps.in.tum.de/hw/himmuc/quick-start/)

   ```bash
   ssh <login>@himmuc.caps.in.tum.de -L localhost:9002:localhost:9002
   ```

7. Run the executables

   Make sure to run the following in the folder `eragp-mandelbrot/backend/build` (or where you placed your executables) on the vmschulz (see example output), *not* on any of the raspberry pis.
   
   If you want to enforce that the host process shares a node with any worker process,
   do set the number of nodes (`-N`) one less than the number of processes (`-n`).
   
   ```bash
   srun -p <odr|rpi> -n <number of workers+1> -N <number of nodes/raspis> -l --multi-prog |<path to eragp-mandelbrot/backend>himmuc/run.conf
   ssh -L 0.0.0.0:9002:localhost:9002 -fN -M -S .tunnel.ssh <odr|rpi><host number>
   ```
   
   Example output:
   ```
   muendler@vmschulz8:~/eragp-mandelbrot/backend/build$ srun -N4 -n5 -l --multi-prog ../himmuc/run.conf
   srun: error: Could not find executable worker
   4: Worker: 4 of 5 on node rpi06
   2: Worker: 2 of 5 on node rpi04
   3: Worker: 3 of 5 on node rpi05
   0: Host: 0 of 5 on node rpi03
   0: Host init 5
   1: Worker: 1 of 5 on node rpi03
   0: Core 1 ready!
   1: Worker 1 is ready to receive Data.
   2: Worker 2 is ready to receive Data.
   0: Listening for connections on to websocket server on 9002
   0: Core 2 ready!
   3: Worker 3 is ready to receive Data.
   0: Core 3 ready!
   4: Worker 4 is ready to receive Data.
   0: Core 4 ready!
   ```
   
   Host number is `03` here, ssh would be told to connect to `rpi03`.

8. How to stop

   ```bash
   ssh -S .tunnel.ssh -O exit rpi<host number>

   # To stop the node allocation
   ps -eo comm,pid | grep srun
   kill <srun pid>
   ```
