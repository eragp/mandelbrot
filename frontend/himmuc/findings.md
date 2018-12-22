# Mandelbrot frontend on the HIMMUC

This is a tutorial on how to run the web frontend of the project
on the HIMMUC cluster.

For this, you need to install npm locally. The pre-built binaries work,
so it is only required to download the binaries, untar them and add the to `$PATH`.

For this, the package was placed inside `~/.eragp-mandelbrot`. 
If you do change this, make sure to replace the path where necessary.

```bash
mkdir ~/.eragp-mandelbrot

wget https://nodejs.org/dist/v10.14.1/node-v10.14.1-linux-x64.tar.xz -o node.tar.xz
tar -xf node.tar.xz # yes this is a x64 binary, yes it works on vmschulz

PATH=$PATH:$HOME/.eragp-mandelbrot/node/bin
export
```

With this set up, we can run the usual npm install and npm start.

```bash
cd eragp-mandelbrot/frontend
npm install

# This command is necessary to run the web frontend
npm start
```
