#!/usr/bin/python3
import subprocess
import argparse
import re
import os
import threading


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
        '-P',
        '--port',
        dest='port',
        help=
        'Set the port on the himmuc to be opened. May need to be configured if more than one user is running this on the himmuc.',
        default=9002)
    parser.add_argument(
        '-p',
        '--partition',
        dest='partition',
        help=
        'Set the partition on which to run the executables. Options are: odr, rpi',
        default="odr")
    args = parser.parse_args()

    argssrun = [
        "srun", "-p", args.partition, "-n{}".format(args.processes), "-N{}".format(args.nodes),
        "--multi-prog", "../himmuc/slurm_run.conf"
    ]

    print_begin(
        "Start mandelbrot with 1 host and {} workers on {} nodes...".format(
            args.processes - 1, args.nodes))
    binary_dir = "{}/eragp-mandelbrot/backend/build".format(
        os.environ.get('HOME'))
    with subprocess.Popen(
            argssrun,
            stdout=subprocess.PIPE,
            universal_newlines=True,
            cwd=binary_dir) as srun:
        print("started mandelbrot")
        _hostpattern = re.compile(r".*Host: \d+ of \d+ on node ({}\d\d).*".format(args.partition))
        host_node = 'ERROR'
        print_begin("Search host node...")
        for line in srun.stdout:
            # Print line so that we can see what is happing
            # print(line[:-1])
            # Try to find the host
            _match = _hostpattern.match(line)
            if _match:
                host_node = _match.group(1)
                break
        print("{} found".format(host_node))
        argsssh = ["ssh", "-L 0.0.0.0:{}:localhost:9002".format(args.port), "-N", host_node]
        print_begin(
            "Establish port {} forwarding to host node {}:9002 ...".format(
                args.port, host_node))
        with subprocess.Popen(argsssh, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL) as sshproc:
            print("established")
            print(
                "System running. Websocket connection to backend is now available at"
            )
            print("\tws://himmuc.caps.in.tum.de:{}".format(args.port))
            
            e = threading.Event()
            e.clear()
            def print_stdout():
                i = 0
                for line in srun.stdout:
                    i = (i+1)%5
                    appendix = " (enter to stop)" if i == 0 else ""
                    print("{}{}".format(line[:-1], appendix)) # print output
                    if e.is_set():
                        break
            
            t = threading.Thread(target=print_stdout)
            t.start()
            try:
                command = input("Press enter (in doubt, twice) to stop - do *not* ctrl-c")
            except KeyboardInterrupt:
                pass
            e.set()

            print_begin("Stopping port forwarding...")
            sshproc.kill()
            try:
                sshproc.wait(5)
                print("stopped ({})".format(sshproc.returncode))
            except subprocess.TimeoutExpired:
                print_begin("failed, sending signal 9...")
                try:
                    sshproc.wait(5)
                    print("stopped ({})".format(sshproc.returncode))
                except subprocess.TimeoutExpired:
                    print("failed")
        print_begin("Stopping mandelbrot host and workers...")
        srun.kill()
        try:
            srun.wait(5)
            print("stopped ({})".format(srun.returncode))
        except subprocess.TimeoutExpired:
            print_begin("failed, sending signal 9...")
            try:
                srun.wait(5)
                print("stopped ({})".format(srun.returncode))
            except subprocess.TimeoutExpired:
                print("failed")
