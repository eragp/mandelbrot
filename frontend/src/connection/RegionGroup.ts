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
    // TODO:
    this.validation = regions[0].region.validation;
    this.maxIteration = regions[0].region.maxIteration;
    this.children = regions.map(r => new Tile(r));
  }

  bounds() {
    return [new Point2D()];
  }

  getChildren() {
    return this.children;
  }
}

class Tile implements RegionGroup {
  public rank: number;
  public computationTime: number;
  public balancer: string;
  public guaranteedDivisor: number;
  public width: number;
  public height: number;
  public minImag: number;
  public maxImag: number;
  public minReal: number;
  public maxReal: number;
  public validation: number;
  public maxIteration: number;
  public hOffset: number;
  public vOffset: number;

  constructor(region: WorkerInfo) {
    this.rank = region.rank;
    this.computationTime = region.computationTime;
    this.balancer = region.region.balancer;
    this.guaranteedDivisor = region.region.guaranteedDivisor;
    this.width = region.region.width;
    this.height = region.region.height;
    this.minImag = region.region.minImag;
    this.maxImag = region.region.maxImag;
    this.minReal = region.region.minReal;
    this.maxReal = region.region.maxReal;
    this.validation = region.region.validation;
    this.maxIteration = region.region.maxIteration;
    this.hOffset = region.region.hOffset;
    this.vOffset = region.region.vOffset;
  }

  /**
   * returns closed polyline of the Region (first point is the same as the last).
   */
  public bounds() {
    return [
      new Point2D(this.maxImag, this.minReal),
      new Point2D(this.maxImag, this.maxReal),
      new Point2D(this.minImag, this.maxReal),
      new Point2D(this.minImag, this.minReal),
      new Point2D(this.maxImag, this.minReal)
    ];
  }

  public getChildren() {
    return null;
  }
}

export const groupRegions = (regions: Regions) => {
  let r = regions.regions;
  if (r.length <= MAX_DISPLAY_REGIONS) {
    return r.map(r => new Tile(r));
  }
  let groupSize = Math.floor(r.length / MAX_DISPLAY_REGIONS);
  let groups: RegionGroup[] = [];
  for (let i = 0; i < Math.floor(r.length / groupSize); i++) {
    let group: WorkerInfo[] = [];
    for (let j = 0; j < groupSize; j++) {
      group.push(r[i * groupSize + j]);
    }
    groups.push(new Group(group));
  }
  return groups;
};
