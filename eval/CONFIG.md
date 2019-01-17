# Evaluation configuration guide

The settings for a configuration run are set in a config.json file.

The number of configurations to be run is `|balancers| * |implementations| * |maxIteration| * |nodeCount| * |pois|`.

## Running a configuration

A configuration in the form of a `.json` file as described below can be run by selecting the
`Start Evaluation` button in the UI and selecting the respective file in the dialog.
When a valid file is selected, it is parsed and each of the specified settings is run.

On each step the currently collected statistics get logged to the console and displayed at the bottom of the modal.
(For every update the output is overwritten)
To copy the output to the clipboard, either copy it from the console, or **tripple click** the generated `json` string at the bottom and press `^C`.

The evaluation window can be exited by clicking anywhere outside the modal dialog.
## Config JSON specification

- `screen`:
  Defines the `width` and `height` in pixels of the viewer.

  ```json
  "screen":{
      "width":1280,
      "height":720
  }
  ```

- `balancers`:
  Defines the balancers to test. (Need to defined in `Constants.ts`)

  ```json
  "balancers":[
      "predictionRecursive"
  ]
  ```

- `implementations`:
  Defines the implementations to be used. (Need to defined in `Constants.ts`)

  ```json
  "implementations":[
      "mandelbrot", "mandelbrot64"
  ]
  ```

- `maxIteration`:
  Defines the maximum number of iterations to use.

  Either one value can be supplied, if only one value should be tested: `[1019]`

  Or if a range should be tested it can be specified with `[min, max, step]`: `[1019, 2000, 200]`

  ```json
  "maxIteration": [1019, 1019, 1]
  ```

- `nodeCount`:
  Defines the number of workers to use. If zero all available workers will be used.

  Either one value can be supplied, if only one value should be tested: `[37]`

  Or if a range should be tested it can be specified with `[min, max, step]`: `[1, 37, 1]`

  ```json
  "nodeCount": [37, 37, 1]
  ```

- `poi`:
  Center view point in the mandelbrot set, as defined in the URL of the region when viewed in the frontend.

  ```json
  "pois": [
  {
      "real": -0.04926860332489014,
      "imag": 0.9887520653946033,
      "zoom": 1
  }, ...
  ```

Example for a full config:

```json
{
  "screen": {
    "width": 1280,
    "height": 720
  },
  "balancers": ["predictionRecursive"],
  "implementations": ["mandelbrot", "mandelbrot64"],
  "maxIteration": [1019, 1019, 1],
  "nodeCount": [37, 37, 1],
  "cluster": "himmuc",
  "description": "Testing different balancer implementations",
  "pois": [
    {
      "real": 0.0,
      "imag": 0.0,
      "zoom": 0
    },
    {
      "real": -0.04926860332489014,
      "imag": 0.9887520653946033,
      "zoom": 1
    },
    {
      "real": -1.4102668762207031,
      "imag": 0.06935050509118243,
      "zoom": 3
    },
    {
      "real": -0.7428462728857994,
      "imag": 0.2057771115182378,
      "zoom": 12
    },
    {
      "real": -0.10538328811526299,
      "imag": 0.9246602339954608,
      "zoom": 12
    },
    {
      "real": -0.11273874342441559,
      "imag": 0.9306590888545648,
      "zoom": 11
    },
    {
      "real": -1.7914929389953613,
      "imag": 0.007840128847674543,
      "zoom": 5
    },
    {
      "real": -1.7767410716041923,
      "imag": 0.008209272087905154,
      "zoom": 15
    }
  ]
}
```
