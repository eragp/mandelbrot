export interface Tour {
  screen: ScreenOpts;
  balancers: string[];
  implementations: string[];
  maxIteration: number[];
  nodeCount: number[];
  runs: number;
  cluster: string;
  description: string;
  pois: PoI[];
}
export interface ScreenOpts {
  width: number;
  height: number;
}
export interface PoI {
  real: number;
  imag: number;
  zoom: number;
}

export interface Output {
  config: Tour;
  datapoints: Run[];
}
export interface Run {
  setting: Setting;
  poi: PoI;
  data: Data;
}

export interface Setting {
  run: number;
  balancer: string;
  implementation: string;
  maxIteration: number;
  nodeCount: number;
}
export interface Data {
  balancer: BalancerData;
  workers: WorkerData[];
}
export interface BalancerData {
  time: number;
  regionCount: number;
}
export interface WorkerData {
  rank: number;
  computationTime: number;
  mpiTime: number;
  drawTime: number;
  pixelCount: number;
  iterationCount: number;
}
