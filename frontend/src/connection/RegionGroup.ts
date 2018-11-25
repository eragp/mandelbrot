import { Regions, WorkerInfo } from "./ComTypes";

const MAX_DISPLAY_REGIONS = 8;
export interface RegionGroup {
  rank: number;
  computationTime: number;
  bounds(): Point[];
}

/**
 * Dynamically groups the returned Regions from the backend for cleaner display to the user.
 */
export default class RegionGroup implements Regions {
  public type: string;
  private regions: WorkerInfo[];

  constructor(regions: Regions) {
      this.type = regions.type;
  }
}
