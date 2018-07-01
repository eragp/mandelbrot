import { registerWorker, registerRegion } from '../connection/WSClient';
import { registerNewRegion } from './TileDisplay';
import {currentTopLeft, currentBottomRight} from './RegionRequest';
import Point from '../misc/Point';

/*
 * this map stores callbacks to render all the tiles requested for leaflet.
 */
const callbacks = new Map();
var topLeft;
var bottomRight;

const handleRegionData = msg => {
    // TODO compute x/y coordinates based on region
    // and invoke tile draw methods
    let tileSize = msg.workerInfo.region.guaranteedDivisor;
    let xStart = topLeft.x + msg.workerInfo.region.hOffset / tileSize;
    let xEnd = xStart + msg.workerInfo.region.width / tileSize;
    let yStart = topLeft.y + msg.workerInfo.region.vOffset / tileSize;
    let yEnd = yStart + msg.workerInfo.region.height / tileSize
    for(let y = yStart; y < yEnd; y++){
        for(let x = xStart; x < xEnd; x++){
            let cb = callbacks.get(coordsToString(x,y,msg.zoom));
            if(cb != null){
                // TODO only pass data of this region
                let roi = new RegionOfInterest(new Point(x, y), new Point(x+tileSize, y+tileSize), 
                    msg.data, msg.workerInfo.region.width, msg.workerInfo.region.height);
                cb(roi);
            }
            else {
                console.log("Region not found for " + new Point(x,y,msg.zoom));
            }
        }
    }
};

const handleNewRegion = map => {
    let bounds = map.getPixelBounds();
    let zoom = map.getZoom();
    let tileSize = map.getTileSize();
    // aka top left
    topLeft = new Point(
      Math.floor(bounds.min.x / tileSize),
      -Math.floor(bounds.min.y / tileSize),
      zoom
    );
    // aka bottom right
    bottomRight = new Point(
      Math.floor(bounds.max.x / tileSize),
      -Math.floor(bounds.max.y / tileSize),
      zoom
    );
};

/**
 * Register yourself at WSClient as regionData observer
 */
registerWorker(handleRegionData);

/**
 * Also observe the current region
 */
registerNewRegion(handleNewRegion);


/**
 *  Registers the tile at coords to be drawn as soon as data is available.
 * @param {*} point coordinates on the tile to be registerd
 * @param {*} draw function expecting data that draws the tile @coords
 */
export const register = (point, draw) => {
    let promise;
    const render = data => {
      promise = new Promise((resolve, error) => {
        try {
          draw(data);
          resolve();
        } catch (err) {
          error(err);
        }
      });
    };
    let coords = coordsToString(point.x, point.y, point.z);
    callbacks.set(coords, render);
    return promise;
  };

function coordsToString(x, y, z) {
    return [x, y, z].join(', ');
}

/**
 * Gibt bei Iteration nur den durch tl,br spezifierten Bereich aus
 */
class RegionOfInterest {
    
    /**
     * tl: topleft: point
     * br: bottomright: point
     */
    constructor(tl,br, data, width,height){
        this.topLeft = tl;
        this.bottomRight = br;
        this.data = data;
        this.width = width;
        this.height = height;

        this.ROIWidth = topLeft.x - bottomRight.x;
        this.ROIHeight = topLeft.y - bottomRight.y;
    }

    /**
     * Returns the correct value of the underlying data array
     * by wrapping x and y in the rectangle of topleft, bottomright
     */
    get(x,y){
        if(x > this.ROIWidth || y > this.ROIHeight){
            console.log("Illegal access");
            return -1;
        }
        let newX = this.topLeft.x + x;
        let newY = this.topLeft.y + y;
        return this.data[y *this.width + x];
    }
}