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
