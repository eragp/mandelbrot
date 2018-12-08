import { Regions, WorkerInfo } from "./ComTypes";
import { Point2D } from "../misc/Point";

const MAX_DISPLAY_REGIONS = 8;
export interface RegionGroup {
  rank: number;
  computationTime: number;
  balancer: string;
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
  public balancer: string;
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
    this.balancer = regions[0].region.balancer;
    this.guaranteedDivisor = regions[0].region.guaranteedDivisor;
    this.validation = regions[0].region.validation;
    this.maxIteration = regions
      .map(r => r.region.maxIteration)
      .reduce((acc, curr) => (curr > acc ? curr : acc));
    this.children = regions.map(r => new Rectangle(r));
    console.log("created region Group with: ");
    console.log(this.children);
  }

  public bounds() {
    let v: Set<Point2D> = new Set();
    this.children.forEach(child => {
      child.bounds().forEach(point => {
        v.add(point);
      });
    });
    let ret: Point2D[] = [];
    v.forEach(p => ret.push(p));
    return ret;
  }

  getChildren() {
    return this.children;
  }
}

class Rectangle implements RegionGroup {
  public rank: number;
  public computationTime: number;
  public balancer: string;
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
    this.balancer = region.region.balancer;
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

  let groupSize = Math.floor(r.length / MAX_DISPLAY_REGIONS);
  let groups = [];
  for (let i = 0; i < Math.floor(r.length / groupSize); i++) {
    let group = [];
    for (let j = 0; j < groupSize; j++) {
      group.push(r[i * groupSize + j]);
    }
    groups.push(new Group(group));
  }
  return groups;
};
