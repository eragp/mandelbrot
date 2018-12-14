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
            "ssh", sshserver_rpi, "eragp-mandelbrot/backend/himmuc/build_mandelbrot.sh"
        ])

    # Start execution on backend
    argsssh = [
        "ssh", sshserver,
        "python3 eragp-mandelbrot/backend/himmuc/start_backend.py {} {}".format(args.processes, args.nodes)
    ]
    with subprocess.Popen(argsssh) as schulz_ssh:
        try:
            schulz_ssh.wait()
        except KeyboardInterrupt:
            # Give it a try...
            print("Really quit?")
            schulz_ssh.wait()