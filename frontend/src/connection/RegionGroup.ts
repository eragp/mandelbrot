import { Regions, WorkerInfo } from "./ComTypes";
import { Point2D } from "../misc/Point";

const MAX_DISPLAY_REGIONS = 8;
export interface RegionGroup {
  rank: number;
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
}

/**
 * Dynamically groups the returned Regions from the backend for cleaner display to the user.
 */
class Group implements RegionGroup {
  public rank: number;
  public computationTime: number;
  public guaranteedDivisor: number;
  public width: number;
  public height: number;
  public validation: number;
  public maxIteration: number;
  public hOffset: number;
  public vOffset: number;
  private children: RegionGroup[];

  constructor(regions: WorkerInfo[]) {
    this.rank = regions[0].rank;
    this.computationTime = regions.map(r => r.computationTime).reduce((acc, curr) => acc + curr);
    this.guaranteedDivisor = regions[0].region.guaranteedDivisor;
    this.validation = regions[0].region.validation;
    this.maxIteration = regions
      .map(r => r.region.maxIteration)
      .reduce((acc, curr) => (curr > acc ? curr : acc));
    this.children = regions.map(r => new Rectangle(r));
  }

  public bounds() {
    let v: Point2D[] = [];
    this.children.forEach(child => {
      child.bounds().forEach(point => {
        if (point.x != 0 && point.y != 0) {
          v.push(point);
        }
      });
    });
    return v;
  }

  getChildren() {
    return this.children;
  }
}

// class CustomSet<V> {
//   // private map: Map<String, V>;
//   private list: V[];

//   constructor() {
//     this.list = [];
//     this[Symbol.iterator] = this.values;
//   }

//   add(item: V): boolean {
//     let unique = true;
//     this.list.forEach(i => i.equals(item) ? unique = false : unique = unique);
//     if (unique) this.list.push(item);
//     return unique;
//   }
//   values() {
//     return this.list;
//   }
//   forEach(fun: (value: V) => void) {
//     this.list.forEach(fun);
//   }
// }

class Rectangle implements RegionGroup {
  public rank: number;
  public computationTime: number;
  public guaranteedDivisor: number;
  public width: number;
  public height: number;
  public validation: number;
  public maxIteration: number;
  public hOffset: number;
  public vOffset: number;

  private minImag: number;
  private maxImag: number;
  private minReal: number;
  private maxReal: number;

  constructor(region: WorkerInfo) {
    this.rank = region.rank;
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

  public bounds() {
    return [
      new Point2D(this.minReal, this.maxImag),
      new Point2D(this.maxReal, this.maxImag),
      new Point2D(this.maxReal, this.minImag),
      new Point2D(this.minReal, this.minImag),
      new Point2D(this.minReal, this.maxImag)
    ];
  }

  public getChildren() {
    return null;
  }
}

export const groupRegions = (regions: Regions): RegionGroup[] => {
  let r = regions.regions;
  if (r.length <= MAX_DISPLAY_REGIONS) {
    return r.map(r => new Rectangle(r));
  }

  let groupSize = Math.ceil(r.length / MAX_DISPLAY_REGIONS);
  let groups = [];
  let i = 0,
    j = 0;
  for (; i < Math.floor(r.length / groupSize); i++) {
    let group = [];
    for (j = 0; j < groupSize; j++) {
      group.push(r[i * groupSize + j]);
    }
    groups.push(new Group(group));
  }
  let remainder = [];
  j = 0;
  while (i * groupSize + j < r.length) {
    remainder.push(r[i * groupSize + j]);
    j++
  }
  groups.push(new Group(remainder));

  console.log("split");
  console.log(regions);
  console.log(groups);
  return groups;
};
