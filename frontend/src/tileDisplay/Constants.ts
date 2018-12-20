export const tileSize = 64;
export const leafletBound = 1024;
export const maxIteration = 1019; // Prime so that %256 never gets black
export const balancer = "naive";

// (real, imaginary) bounds of the complex plane
// x/-x and y/-y have to be the same (symmetric)
export const bounds = [4, 4];
interface Balancer {
  key: string;
  title: string;
}
export const balancers: Balancer[] = [
  {
    key: "naive",
    title: "Naive Balancer"
  },
  {
    key: "column",
    title: "Column Balancer"
  },
  {
    key: "prediction",
    title: "Prediction Balancer"
  },
  {
    key: "predictionRecursive",
    title: "Recursive Prediction Balancer"
  }
];
