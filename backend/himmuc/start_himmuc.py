#!/usr/bin/python3
import subprocess
import argparse
import os

arguments = [{
    'dest': 'username',
    'help': 'Rechnerkennung',
    'type': str,
}, {
    'dest': 'processes',
    'help': 'Number of seperate processes to start',
    'type': int,
    'default': 10,
}, {
    'dest': 'nodes',
    'help': 'Number of seperate nodes to start',
    'type': int,
    'default': 9,
}]
if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        'start_himmuc',
        description='Connect to himmuc and run eragp-mandelbrot from nodes')
    for arg in arguments:
        parser.add_argument(**arg)
    parser.add_argument(
        '-b',
        '--build',
        action='store_const',
        const=True,
        dest='build',
        help=
        'Issue a build process on the backend (i.e. if you changed the src)',
        default=False)
    parser.add_argument(
        '-r',
        '--no-rsync',
        action='store_const',
        const=True,
        dest='no_rsync',
        help=
        'Tell the script not to use rsync. Note that for this, you need to copy the eragp-mandelbrot project folder into the vmschulz home directory manually BEFORE running this script!',
        default=False)
    parser.add_argument(
        '-P',
        '--port',
        dest='port',
        help=
        'Set the port on the himmuc to be opened',
        default=9002)
    parser.add_argument(
        '-p',
        '--partition',
        dest='partition',
        help=
        'Set the partition on which to run the executables. Options are: odr, rpi',
        default="odr")
    args = parser.parse_args()
    sshserver = "{}@himmuc.caps.in.tum.de".format(args.username)
    backend_path = "eragp-mandelbrot/backend"
    # -> run python script on vmschulz
    if not args.no_rsync:
        print("Uploading backend... ", end=" ", flush=True)
        subprocess.run([
            "ssh", sshserver, "mkdir {}".format(backend_path)
        ], stderr=subprocess.DEVNULL)
        subprocess.run([
            "rsync", "-Prua", '../backend', "{}:eragp-mandelbrot".format(sshserver)
        ])
        print("done")
    else:
        print("Not uploading backend - make sure to have copied the project to your home directory already!")
    
    # issue build on rpi
    if args.build:
        sshserver_rpi = "{}@sshgate-gepasp.in.tum.de".format(args.username)
        # check if boost has to be built
        try:
            print("Attempting to install libraries on backend")
            # Attempt to create boost install folder
            # error is thrown when return code is not 0 <=> dir exists
            library_folder_exists = subprocess.run([
                "ssh", sshserver, "mkdir .eragp-mandelbrot/local"
            ], stderr=subprocess.DEVNULL, check=True)
            subprocess.run([
                "ssh", sshserver, "mkdir .eragp-mandelbrot/install"
            ], stderr=subprocess.DEVNULL)
            subprocess.run([
                "ssh", sshserver, "eragp-mandelbrot/backend/himmuc/install_libs.sh"
            ])
        except subprocess.CalledProcessError:
            print("Backend libraries already installed")
        # Build mandelbrot
        subprocess.run([
            "ssh", sshserver, "mkdir eragp-mandelbrot/backend/build"
        ], stderr=subprocess.DEVNULL)
        subprocess.run([
            "ssh", sshserver_rpi, 
        ], input="eragp-mandelbrot/backend/himmuc/build_mandelbrot.sh; exit\n", universal_newlines=True,
        stderr=subprocess.DEVNULL)

    # Start execution on backend
    # also forward local port 9002 to the chosen port on the himmuc
    argsssh = [
        "ssh", sshserver, "-L localhost:9002:localhost:{}".format(args.port),
        "python3 eragp-mandelbrot/backend/himmuc/start_backend.py {} {} --partition {} --port {}".format(args.processes, args.nodes, args.partition, args.port)
    ]
    with subprocess.Popen(argsssh) as schulz_ssh:
        schulz_ssh.wait()