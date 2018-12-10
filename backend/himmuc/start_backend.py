#!/usr/bin/python3
import subprocess
import argparse
import re
import os


def print_begin(*args, **kwargs):
    print(*args, end=' ', flush=True, **kwargs)


arguments = [{
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
        'start_backend',
        description='Run nodes from slurm implementing device (vmschulz)')
    for arg in arguments:
        parser.add_argument(**arg)
    parser.add_argument(
        '-p',
        '--pre_built',
        action='store_const',
        const=True,
        dest='pre_built',
        help=
        'Use prebuilt binaries that lie in same directory as this script or use binaries in ~/eragp-mandelbrot/backend/build.',
        default=False)
    args = parser.parse_args()

    argssrun = [
        "srun", "-n{}".format(args.processes), "-N{}".format(args.nodes),
        "--multi-prog", "run.conf"
    ]

    print_begin(
        "Start mandelbrot with 1 host and {} workers on {} nodes...".format(
            args.processes - 1, args.nodes))
    binary_dir = "{}/eragp-mandelbrot/backend/build".format(
        os.environ.get('HOME')) if not args.pre_built else os.environ.get(
            'HOME')
    with subprocess.Popen(
            argssrun,
            stdout=subprocess.PIPE,
            universal_newlines=True,
            cwd=binary_dir) as srun:
        print("started mandelbrot")
        _hostpattern = re.compile(r".*Host: \d+ of \d+ on node (rpi\d\d).*")
        host_node = 'ERROR'
        for line in srun.stdout:
            # Print line so that we can see what is happing
            # print(line[:-1])
            # Try to find the host
            _match = _hostpattern.match(line)
            if _match:
                host_node = _match.group(1)
                break
        argsssh = ["ssh", "-L 0.0.0.0:9002:localhost:9002", host_node]
        print_begin(
            "Establish port 9002 forwarding to host node {} ...".format(
                host_node))
        with subprocess.Popen(argsssh, stdout=subprocess.DEVNULL) as sshproc:
            print("port forwarding established")
            print(
                "System running. Websocket connection to backend is now available at"
            )
            print("ws://himmuc.caps.in.tum.de:9002")
            try:
                command = input("Press enter to stop ")
            except KeyboardInterrupt:
                print("Naughty you, this was not enter!")
            print_begin("Stopping port forwarding...")
            sshproc.kill()
        print("stopped")
        print_begin("Stopping mandelbrot host and workers...")
        srun.kill()
        print("stopped")
