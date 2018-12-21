/**
 * Objects used for communicating with the backend.
 */

export interface Request {
  balancer: string;
  guaranteedDivisor: number;
  width: number;
  height: number;
  minImag: number;
  maxImag: number;
  minReal: number;
  maxReal: number;
  validation: number;
  maxIteration: number;
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

export const regionEquals = (a: Region, b: Region) => {
  return (
    a.guaranteedDivisor === b.guaranteedDivisor &&
    a.width === b.width &&
    a.height === b.height &&
    a.minImag === b.minImag &&
    a.maxImag === b.maxImag &&
    a.minReal === b.minReal &&
    a.maxReal === b.maxReal &&
    a.validation === b.validation &&
    a.maxIteration === b.maxIteration &&
    a.hOffset === b.hOffset &&
    a.vOffset === b.vOffset
  );
};

export interface WorkerInfo {
  rank: number;
  computationTime: number;
  region: Region;
}

export interface Regions {
  type: string;
  regions: WorkerInfo[];
}

export interface RegionData {
  data: number[];
  type: string;
  workerInfo: WorkerInfo;
}
