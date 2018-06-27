# General
Die JS UI was created with react.
NPM is used as package manager.

# Client-Server Communicaiton
Client requests two types of resources from the server. Regions and
Tiles.

## Region
HTTP GET Parameters:

```markdown
- zoom:         current region level zoom factor
- topLeftX:     X index from the origin of the last visible top left tile
- topLeftY:     Y index from the origin of the last visible top left tile
- bottomRightX: X index from the origin of the last visible top right tile
- bottomRightY: Y index from the origin of the last visible top right tile
- balancer:     type of LoadBalancer to choose {1}
```

## Tile

HTTP GET Parameters:

```markdown
- x:    x index form the origin of the current tile
- y:    y index form the origin of the current tile
- z:    zoom factor of the current tile 
- size: tile size in pixels (constant for all tiles)
```

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
