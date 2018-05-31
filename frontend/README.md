# General
Die JS UI was created with react.
NPM is used as package manager.

# Getting started
Download all required libraries:

```bash
$ npm install
```

Current workaround:
Manually place leaflet-src.js from the forked version in the node_modules folder.
```bash
$ wget https://raw.githubusercontent.com/nielstron/Leaflet/fixed-ratio-selection-rectangle-build/dist/leaflet-src.js -O ./node_modules/leaflet/dist/leaflet-src.js
```

Start the local server:

```bash
$ npm start
```

Compile the javaScript:

```bash
$ npm run build
```
