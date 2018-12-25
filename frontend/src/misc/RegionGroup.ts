import { WorkerInfo, Region } from "../connection/ExchangeTypes";
import { Point2D } from "./Point";
import { MAX_DISPLAY_REGIONS } from "../Constants";

export interface RegionGroup {
  id: number;
  computationTime: number;
  guaranteedDivisor: number;
  width: number;
  height: number;
  validation: number;
  maxIteration: number;
  hOffset: number;
  vOffset: number;
  /**
   * returns closed polyline of the Region (first point is the same as the last).
   */
  bounds(): Point2D[];
  getChildren(): RegionGroup[] | null;
  getLeafs(): Rectangle[];
  getRanks(): number[];
  isGroup(): boolean;
}

/**
 * Dynamically groups the returned Regions from the backend for cleaner display to the user.
 */
class Group implements RegionGroup {
  public id: number;
  public computationTime: number;
  public guaranteedDivisor: number;
  public width: number;
  public height: number;
  public validation: number;
  public maxIteration: number;
  public hOffset: number;
  public vOffset: number;
  private children: RegionGroup[];

  constructor(regions: WorkerInfo[], groupID: number) {
    this.id = groupID;
    this.computationTime = regions.map(r => r.computationTime).reduce((acc, curr) => acc + curr);
    this.guaranteedDivisor = regions[0].region.guaranteedDivisor;
    this.validation = regions[0].region.validation;
    this.maxIteration = regions
      .map(r => r.region.maxIteration)
      .reduce((acc, curr) => (curr > acc ? curr : acc));
    this.children = regions.map(r => new Rectangle(r));
  }

  /**
   * Convex hull of all children computed with Graham scan Algorithm:
   * https://en.wikipedia.org/wiki/Graham_scan
   */
  public bounds() {
    let points = this.children.map(c => c.bounds()).reduce((acc, curr) => acc.concat(curr));
    console.log(`Group ${this.id} child points:`);
    console.log(points);
    // Find the pivot with min y value
    let start = points.reduce((acc, curr) =>
      // min y, if acc.y == curr.y find min x
      curr.y < acc.y || (curr.y === acc.y && curr.x < acc.x) ? curr : acc
    );

    const angle = (p: Point2D) => Math.atan2(p.y - start.y, p.x - start.x);
    // sort by (angle, x)
    points.sort((a, b) => (angle(a) === angle(b) ? a.x - b.x : angle(a) - angle(b)));

    // Adding points to the result if they "turn left"
    let convex = [start],
      len = 1;
    for (let i = 1; i < points.length; i++) {
      let a = convex[len - 2],
        b = convex[len - 1],
        c = points[i];
      while (
        (len === 1 && b.x === c.x && b.y === c.y) ||
        (len > 1 && (b.x - a.x) * (c.y - a.y) <= (b.y - a.y) * (c.x - a.x))
      ) {
        len--;
        b = a;
        a = convex[len - 2];
      }
      convex[len++] = c;
    }
    convex.length = len;
    convex.push(convex[0]);
    console.log(`Group ${this.id} convex hull:`);
    console.log(convex);

    // add points for concave hull
    let concave: Point2D[] = [];
    for (let i = 0; i < convex.length - 1; i++) {
      let p0 = convex[i];
      let p1 = convex[i + 1];
      concave.push(p0);
      if (p0.x !== p1.x && p0.y !== p1.y) {
        let d0 = new Point2D(p0.x, p1.y);
        let d1 = new Point2D(p1.x, p0.y);
        if (points.some(p => p.equals(d0))) {
          concave.push(d0);
        } else {
          concave.push(d1);
        }
      }
    }
    concave.push(convex[convex.length - 1]);
    console.log(`Group ${this.id} concave hull:`);
    console.log(concave);
    return concave;
  }

  public getChildren() {
    return this.children;
  }

  public isGroup() {
    return true;
  }

  public getLeafs() {
    let leafs: Rectangle[] = [];
    this.getChildren().forEach(child => {
      leafs = leafs.concat(child.getLeafs());
    });
    return leafs;
  }

  public getRanks() {
    return this.children.map(c => c.getRanks()).reduce((acc, curr) => acc.concat(curr));
  }
}

class Rectangle implements RegionGroup {
  id: number;
  computationTime: number;
  guaranteedDivisor: number;
  width: number;
  height: number;
  validation: number;
  maxIteration: number;
  hOffset: number;
  vOffset: number;

  private minImag: number;
  private maxImag: number;
  private minReal: number;
  private maxReal: number;

  constructor(region: WorkerInfo) {
    this.id = region.rank;
    this.computationTime = region.computationTime;
    this.guaranteedDivisor = region.region.guaranteedDivisor;
    this.width = region.region.width;
    this.height = region.region.height;
    this.validation = region.region.validation;
    this.maxIteration = region.region.maxIteration;
    this.hOffset = region.region.hOffset;
    this.vOffset = region.region.vOffset;

    this.minImag = region.region.minImag;
    this.maxImag = region.region.maxImag;
    this.minReal = region.region.minReal;
    this.maxReal = region.region.maxReal;
  }

  bounds() {
    return [
      new Point2D(this.minReal, this.maxImag),
      new Point2D(this.maxReal, this.maxImag),
      new Point2D(this.maxReal, this.minImag),
      new Point2D(this.minReal, this.minImag),
      new Point2D(this.minReal, this.maxImag)
    ];
  }

  getChildren() {
    return null;
  }

  isGroup() {
    return false;
  }

  public getLeafs() {
    return [this];
  }

  getRanks() {
    return [this.id];
  }
}

export const groupRegions = (r: WorkerInfo[]): RegionGroup[] => {
  console.log(r);
  if (r.length <= MAX_DISPLAY_REGIONS) {
    return r.map(r => new Rectangle(r));
  }
  let groupSize = Math.ceil(r.length / MAX_DISPLAY_REGIONS);
  let groups: Group[] = [];
  let groupID = 1;

  let rects = r;
  do {
    let g = expandGroup(rects[0], rects, groupSize);
    rects = sub(rects, g);
    groups.push(new Group(g, groupID++));
  } while (rects.length != 0);
  console.log(groups);
  return groups;
};

const regionEquals = (a: Region, b: Region) =>
  a.minReal === b.minReal &&
  a.maxReal === b.maxReal &&
  a.minImag === b.minImag &&
  a.maxImag === b.maxImag;

const sub = (a: WorkerInfo[], b: WorkerInfo[]) =>
  a.filter(i => !b.some(j => regionEquals(i.region, j.region)));

const expandGroup = (start: WorkerInfo, rects: WorkerInfo[], size: number): WorkerInfo[] => {
  const getBounds = (r: Region) => {
    return [
      new Point2D(r.minReal, r.minImag),
      new Point2D(r.minReal, r.maxImag),
      new Point2D(r.maxReal, r.minImag),
      new Point2D(r.maxReal, r.maxImag)
    ];
  };
  const hasOverlap = (a: Point2D[], b: Point2D[]) => a.some(i => b.some(j => i.equals(j)));
  let n = rects.filter(
    w =>
      hasOverlap(getBounds(start.region), getBounds(w.region)) &&
      !regionEquals(start.region, w.region)
  );
  if (n.length == 0) {
    return [];
  } else if (n.length < size) {
    let expanded: WorkerInfo[] = [];
    for (let i = 0; i < n.length; i++) {
      expanded = expandGroup(n[i++], sub(rects, n), size - n.length);
      if (expanded.length != 0) break;
    }
    return n.concat(expanded);
  } else {
    return n.slice(0, size);
  }
};
