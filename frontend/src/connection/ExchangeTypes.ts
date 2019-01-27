/**
 * Objects used for communicating with the backend.
 */

export interface Request {
  type: string;
  region: Region;
  balancer: string;
  fractal: string;
  nodes: number;
  predictionAccuracy: number;
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
  fractal: string;
  regionCount: number;
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

export const regionEquals = (r1: Region, r2: Region) => {
  return (
    r1.fractal.toLowerCase() === r2.fractal.toLowerCase() &&
    r1.regionCount === r2.regionCount &&
    r1.validation === r2.validation &&
    r1.hOffset === r2.hOffset &&
    r1.vOffset === r2.vOffset &&
    r1.width === r2.width &&
    r1.height === r2.height &&
    r1.minImag === r2.minImag &&
    r1.maxImag === r2.maxImag &&
    r1.minReal === r2.minReal &&
    r1.maxReal === r2.maxReal
  );
};

export const workerInfoEquals = (w1: WorkerInfo, w2: WorkerInfo) => {
  return w1.rank === w2.rank && regionEquals(w1.region, w2.region);
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
