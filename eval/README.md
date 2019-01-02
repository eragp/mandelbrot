# Plotter for evaluation data

## Installation

To run the plotter you need to install `anaconda` to create the virtual python environment. (miniconda is sufficient for this project).

1. Install anaconda by following the instructions [here](https://conda.io/docs/user-guide/install/index.html)

After the installation of anaconda you can proceed to create the virtual environment

```bash
$ conda create -n mandelbrot_eval python=3.7.1 ipykernel pip
Solving environment: done

## Package Plan ##

  environment location: /Users/databook/miniconda3/envs/mandelbrot_eval

  added / updated specs:
    - ipykernel
    - pip
    - python=3.7.1


The following NEW packages will be INSTALLED:

...

Proceed ([y]/n)? y

Preparing transaction: done
Verifying transaction: done
Executing transaction: done
#
# To activate this environment, use
#
#     $ conda activate mandelbrot_eval
#
# To deactivate an active environment, use
#
#     $ conda deactivate
```

Activate the virtual environment (needs to be done **before** starting the notebook):

```bash
$ conda activate mandelbrot_eval
```

When the env is activated for the first time, the installation script `./install.sh` needs to be executed once.

```bash
$ ./install.sh
...
Successfully installed numpy-1.15.4
Installed kernelspec python3 in /usr/local/share/jupyter/kernels/python3
Installed kernelspec python3 in /usr/local/share/jupyter/kernels/python3
```

## Running the notebook
To start the notebook, run the following command.

```bash
$ jupyter notebook
```

In the opened browser tab, you can select the file you want to edit.

## Stopping the notebook
The notebook can be stopped by pressing ^C on the console.
To deactivate the virtual environment (when done with working on the plotter):

```bash
$ conda deactivate
```
