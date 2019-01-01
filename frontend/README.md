# General
Die JS UI was created with react.
NPM is used as package manager.

# Getting started
Download all required libraries:

```bash
$ npm install
```

Start the local server:

```bash
$ npm start
```

Compile the javaScript:

```bash
$ npm run build
```

# Client-Server Communication
The connection between front- and backend is implemented with a websockets connection, that allows us to send data asynchronously between the two.

## Region request

Example:
```js
{  
   "region":{  
      "minReal":-2,
      "maxImag":1.75,
      "maxReal":2,
      "minImag":-1.75,
      "width":1024,
      "height":896,
      "hOffset":0,
      "vOffset":0,
      "validation":0,
      "guaranteedDivisor":64,
      "maxIteration":1019
   },
   "type":"regionRequest",
   "balancer":"naive",
   "fractal":"mandelbrotsimd32"
}
```

## Region response

Example:
```js
{
   "type":"region",
   "regionCount":37,
   "regions":[
      {
         "rank":1,
         "computationTime":0,
         "region":{
            "minReal":-2.0,
            "maxImag":1.75,
            "maxReal":2.0,
            "minImag":1.5,
            "width":1024,
            "height":64,
            "hOffset":0,
            "vOffset":0,
            "maxIteration":1019,
            "validation":0,
            "guaranteedDivisor":64
        }
      },
    // ...
   ]
}
```

## Region data

Example:

```js
{
   "workerInfo":{
      "rank":5,
      "computationTime":94079,
      "region":{
         "minReal":-2.0,
         "maxImag":0.75,
         "maxReal":2.0,
         "minImag":0.5,
         "width":1024,
         "height":64,
         "hOffset":0,
         "vOffset":256,
         "maxIteration":1019,
         "validation":0,
         "guaranteedDivisor":64
      }
   },
   "data":[ ... ],
   "type":"regionData"
}
```