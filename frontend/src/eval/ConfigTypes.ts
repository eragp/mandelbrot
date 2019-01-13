export interface Tour {
  screen: ScreenOpts;
  settings: Setting[];
  cluster: string;
  description: string;
  pois: PoI[];
}
export interface Setting {
  balancer: string;
  implementation: string;
  nodes: number;
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
  data: Timings;
}
export interface Timings {
  balancer: BalancerTime;
  workers: WorkerTime[];
}
export interface BalancerTime {
  time: number;
  regionCount: number;
}
export interface WorkerTime {
  rank: number;
  computationTime: number;
  mpiTime: number;
  drawTime: number;
}
