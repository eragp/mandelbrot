import { WorkerInfo, Region, regionEquals, isEmptyRegion } from "../connection/ExchangeTypes";
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
   * returns polygon of the Region
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
   * Concave hull of all children computed with Graham scan Algorithm:
   * https://en.wikipedia.org/wiki/Graham_scan
   * Then all non 90Deg angles are removed.
   */
  public bounds() {
    let points = this.children.map(c => c.bounds()).reduce((acc, curr) => acc.concat(curr));
    // Find the pivot with min y value
    let start = points.reduce((acc, curr) =>
      // min y, if acc.y == curr.y find min x
      curr.y < acc.y || (curr.y === acc.y && curr.x < acc.x) ? curr : acc
    );

    const angle = (p: Point2D) => Math.atan2(p.y - start.y, p.x - start.x);
    // sort by (angle, x)
    points.sort((a, b) => (angle(a) === angle(b) ? a.x - b.x : angle(a) - angle(b)));

    // Adding points to the result if they "turn left"
    const convex = [start];
    let len = 1;
    for (let i = 1; i < points.length; i++) {
      let a = convex[len - 2];
      let b = convex[len - 1];
      let c = points[i];
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

    // add points for concave hull
    let concave: Point2D[] = [];
    for (let i = 0; i < convex.length - 1; i++) {
      let p0 = convex[i];
      let p1 = convex[i + 1];
      concave.push(p0);
      // if a triangle has been created, add the point left / right
      // of the edge to create a 90Deg corner
      if (p0.x !== p1.x && p0.y !== p1.y) {
        let d0 = new Point2D(p0.x, p1.y);
        let d1 = new Point2D(p1.x, p0.y);
        concave.push(points.some(p => p.equals(d0)) ? d0 : d1);
      }
    }
    concave.push(convex[convex.length - 1]);
    return concave;
  }

  public getChildren() {
    return this.children;
  }

  public isGroup() {
    return true;
  }

  public getLeafs() {
    return this.children.map(c => c.getLeafs()).reduce((acc, curr) => acc.concat(curr));
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
      new Point2D(this.minReal, this.minImag)
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
  if (r.length <= MAX_DISPLAY_REGIONS) {
    return r.map(r => new Rectangle(r));
  }
  // If there are empty regions, create a whole group for them
  const emptyRegions: WorkerInfo[] = [];
  r.forEach(w => {
      if(isEmptyRegion(w.region)){
        emptyRegions.push(w);
      }
  });;
  r = r.filter(w => !isEmptyRegion(w.region));

  const groupSize = Math.ceil(r.length / MAX_DISPLAY_REGIONS);
  const groups: WorkerInfo[][] = [];
  let groupID = 1;
  const center = (r: Region) =>
    new Point2D((r.minReal + r.maxReal) / 2, (r.minImag + r.maxImag) / 2);
  const sub = (a: WorkerInfo[], b: WorkerInfo[]) =>
    a.filter(i => !b.some(j => regionEquals(i.region, j.region)));

  let rect = r;
  // sort regions by midpoint
  rect.sort((a, b) =>
    center(a.region).x - center(b.region).x !== 0
      ? center(b.region).x - center(a.region).x
      : center(a.region).y - center(b.region).y
  );
  while (rect.length > 0) {
    const w = rect.slice(0, Math.min(groupSize, rect.length));
    rect = sub(rect, w); // I guess this leads to errors on empty regions
    groups.push(w);
  }
  if(emptyRegions.length > 0){
      groups.push(emptyRegions);
  }
  // console.log(groups);
  return groups.map(g => new Group(g, groupID++));
};
