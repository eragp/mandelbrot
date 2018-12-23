export const tileSize = 64;
export const leafletBound = 1024;
export const maxIteration = 1019; // Prime so that %256 never gets black
export const balancer = "naive";

// Maximum number of regions to display to the user.
// If more are delivered by the backend, grouping will occur.
export const MAX_DISPLAY_REGIONS = 4;

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
    key: "prediction",
    title: "Prediction Balancer"
  },
  {
    key: "naiveRecursive",
    title: "Recursive Naive Balancer"
  },
  {
    key: "predictionRecursive",
    title: "Recursive Prediction Balancer"
  },
  {
    key: "column",
    title: "Column Balancer"
  },
];
