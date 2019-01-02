/**
 * Objects used for communicating with the backend.
 */

export interface Request {
  type: string;
  region: Region;
  balancer: string;
  fractal: string;
}

export interface Region {
  guaranteedDivisor: number;
  width: number;
  height: number;
  minImag: number;
  maxImag: number;
  minReal: number;
  maxReal: number;
  validation: number;
  maxIteration: number;
  hOffset: number;
  vOffset: number;
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

export interface WorkerInfo {
  rank: number;
  computationTime: number;
  mpiTime: number;
  region: Region;
}

export interface Regions {
  type: string;
  regionCount: number;
  balancerTime: number;
  regions: WorkerInfo[];
}

export interface RegionData {
  data: number[];
  type: string;
  workerInfo: WorkerInfo;
}
