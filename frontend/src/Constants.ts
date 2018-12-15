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
