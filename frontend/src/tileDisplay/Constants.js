export const tileSize = 64;
export const maxIteration = 1019; // Prime so that %256 never gets black
export const balancer = 'naive';

// (real, imaginary) bounds of the complex plane
// x/-x and y/-y have to be the same (symmetric)
export const bounds = [4.0, 4.0];
