#!/usr/bin/python3
import subprocess
import argparse

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
        '--pre_built',
        '-p',
        action='store_const',
        const=True,
        dest='pre_built',
        help=
        'Use pre puilt executables from local git clone. Otherwise uses executables located on the raspi at ~/eragp-mandelbrot/backend/build',
        default=True)
    args = parser.parse_args()
    # meh.. would rather start the process in the shell on the target
    # -> run python script on vmschulz
    print("Uploading scripts and executables... ", end=" ", flush=True)
    print("start_backend.py", end=" ", flush=True)
    subprocess.run([
        "rsync", "-ua", "start_backend.py",
        "{}@himmuc.caps.in.tum.de:".format(args.username)
    ])
    if args.pre_built:
        print("host worker", end=" ", flush=True)
        subprocess.run([
            "rsync", "-ua", "../build/host",
            "{}@himmuc.caps.in.tum.de:".format(args.username)
        ])
        subprocess.run([
            "rsync", "-ua", "../build/worker",
            "{}@himmuc.caps.in.tum.de:".format(args.username)
        ])
    print("run.conf", end=" ", flush=True)
    subprocess.run(
        ["rsync", "-ua", "run.conf", "{}@himmuc.caps.in.tum.de:".format(args.username)])
    print()

    argsssh = [
        "ssh", "{}@himmuc.caps.in.tum.de".format(args.username),
        "python3 {}start_backend.py {} {} {}".format(
            ('~/eragp-mandelbrot/backend/himmuc/'
             if not args.pre_built else ''), args.processes, args.nodes,
            ('-p' if args.pre_built else ''))
    ]
    with subprocess.Popen(argsssh) as schulz_ssh:
        try:
            schulz_ssh.wait()
        except KeyboardInterrupt:
            # Give it a try...
            print("Really quit?")
            schulz_ssh.wait()