#!/usr/bin/python3
import subprocess
import argparse

arguments = [
    {
        'dest': 'username',
        'help': 'Rechnerkennung',
        'type': str,
    },
    {
        'dest': 'processes',
        'help': 'Number of seperate processes to start',
        'type': int,
        'default': 10
    },
    {
        'dest': 'nodes',
        'help': 'Number of seperate nodes to start',
        'type': int,
        'default': 9
    }
]
if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        'start_himmuc',
        description='Connect to himmuc and run eragp-mandelbrot from nodes'
    )
    for arg in arguments:
        parser.add_argument(**arg)
    args = parser.parse_args()
    # meh.. would rather start the process in the shell on the target
    # -> run python script on vmschulz
    argsssh = ["ssh", "{}@himmuc.caps.in.tum.de".format(args.username), "python3 ~/eragp-mandelbrot/backend/himmuc/start_backend.py {} {}".format(args.processes, args.nodes)]

    with subprocess.Popen(argsssh) as schulz_ssh:
        schulz_ssh.wait()