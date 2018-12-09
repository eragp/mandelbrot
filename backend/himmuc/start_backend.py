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

    print("Start mandelbrot host and workers...")
    with subprocess.Popen(argssrun, stdout=subprocess.PIPE, universal_newlines=True, cwd="{}/eragp-mandelbrot/backend/build".format(os.environ.get('HOME'))) as srun:
        print("Started mandelbrot")
        hostpattern = re.compile(r".*Host: \d+ of \d+ on node (rpi\d\d).*")
        for line in srun.stdout:
            # Print line so that we can see what is happing
            # print(line[:-1])
            # Try to find the host
            match_ = hostpattern.match(line)
            if match_:
                break
        argsssh = ["ssh", "-L 0.0.0.0:9002:localhost:9002", match_.group(1)]
        print("Establish port forwarding to host node...")
        with subprocess.Popen(argsssh, stdout=subprocess.DEVNULL) as sshproc:
            print("Port forwarding established")
            print("System running")
            command = input("Press enter to stop")
            print("Stopping port forwarding...")
            sshproc.kill()
        print("Stopped")
        print("Stopping mandelbrot host and workers")
        srun.kill()
        print("Stopped")
