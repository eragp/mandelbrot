import { Regions, WorkerInfo, Region } from "../connection/ExchangeTypes";
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
  getRanks(): number[];
  isGroup(): boolean;
}

/**
 * Dynamically groups the returned Regions from the backend for cleaner display to the user.
 */
class Group implements RegionGroup {
  id: number;
  computationTime: number;
  guaranteedDivisor: number;
  width: number;
  height: number;
  validation: number;
  maxIteration: number;
  hOffset: number;
  vOffset: number;
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
   * bound of a group will return a rectangle fitting all of it's children
   */
  bounds() {
    let tl = this.children[0].bounds()[0],
      br = this.children[this.children.length - 1].bounds()[2];
    return [
      new Point2D(tl.x, tl.y),
      new Point2D(br.x, tl.y),
      new Point2D(br.x, br.y),
      new Point2D(tl.x, br.y),
      new Point2D(tl.x, tl.y)
    ];
  }

  getChildren() {
    return this.children;
  }

  isGroup() {
    return true;
  }
  getRanks() {
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

  getRanks() {
    return [this.id];
  }
}

export const isEmptyRegion = (region: Region) => {
  return (
    region.width === 0 &&
    region.height === 0 &&
    region.minImag === 0 &&
    region.maxImag === 0 &&
    region.minReal === 0 &&
    region.maxReal === 0 &&
    region.hOffset === 0 &&
    region.vOffset === 0
  );
};

export const groupRegions = (regions: Regions): RegionGroup[] => {
  let r = regions.regions.filter(r => !isEmptyRegion(r.region));
  if (r.length <= MAX_DISPLAY_REGIONS) {
    return r.map(r => new Rectangle(r));
  }
  let groupSize = Math.ceil(r.length / MAX_DISPLAY_REGIONS);
  let groups = [];
  let groupID = 1;
  let i = 0,
    j = 0;
  for (; i < Math.floor(r.length / groupSize); i++) {
    let group = [];
    for (j = 0; j < groupSize; j++) {
      group.push(r[i * groupSize + j]);
    }
    groups.push(new Group(group, groupID++));
  }
  {
    let remainder = [];
    j = 0;
    while (i * groupSize + j < r.length) {
      remainder.push(r[i * groupSize + j]);
      j++;
    }
    if (remainder.length > 0) groups.push(new Group(remainder, groupID++));
  }
  return groups;
};
