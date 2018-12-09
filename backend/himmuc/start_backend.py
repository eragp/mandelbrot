#!/usr/bin/python3
import subprocess
import argparse
import re
import os

arguments = [
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
        'start_backend',
        description='Run nodes from slurm implementing device (vmschulz)'
    )
    for arg in arguments:
        parser.add_argument(**arg)
    args = parser.parse_args()

    argssrun = ["srun", "-n{}".format(args.processes), "-N{}".format(args.nodes), "--multi-prog", "../himmuc/run.conf"]

    with subprocess.Popen(argssrun, stdout=subprocess.PIPE, universal_newlines=True, cwd="{}/eragp-mandelbrot/backend/build".format(os.environ.get('HOME'))) as srun:
        hostpattern = re.compile(r"Host: \d+ of \d+ on node (rpi\d\d)")
        for line in srun.stdout:
            # Print line so that we can see what is happing
            print(line)
            # Try to find the host
            match_ = hostpattern.match(line)
            if match_:
                argsssh = ["ssh", "-L 0.0.0.0:9002:localhost:9002", match_.group(1)]
                with subprocess.Popen(argsssh, stdout=subprocess.DEVNULL) as sshproc:
                    print("Enter to exit")
                    command = input()
                    srun.kill()
                    sshproc.kill()
