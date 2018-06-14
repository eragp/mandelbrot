# Allgemein
Die JS UI ist mit dem framework react erstellt.
Als dependency management habe ich npm verwendet.

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
Herunterladen aller benötigten libraries:

```bash
$ npm install
```

Starten des lokalen Servers:

```bash
$ npm start
```

Kompilieren des JavaScript:

```bash
$ npm run build
```
