#include "PredictionBalancer.h"
#include "TileInfo.h"
#include "Fractal.h"

// TileInfo* PredictionBalancer::balanceLoad(TileInfo tile, int nodeCount)
// {
// 	TileInfo lowRes = tile;
// 	lowRes.xRes = tile.xRes / 16;
// 	lowRes.yRes = tile.yRes / 16;
// 	int maxIteration = 200;
// 	// TileInfo* predictionTiles = NaiveBalancer::balanceLoad(lowRes, nodeCount); // Do this with MPI?
// 	int n[lowRes.xRes][lowRes.yRes];
// 	for (int x = 0; x < lowRes.yRes; x++) {
// 		for (int y = 0; y < lowRes.yRes; y++) {
// 			n[x][y] = f->calculateFractal(Fractal::xToReal(x, lowRes.maxReal, lowRes.minReal, lowRes.xRes), Fractal::yToImaginary(y, lowRes.maxImaginary, lowRes.minImaginary, lowRes.yRes), maxIteration);
// 		}
// 	}
// 	// method stub
// 	TileInfo* allTiles = new TileInfo[nodeCount];
// 	return allTiles;
// }

// PredictionBalancer* PredictionBalancer::create(Fractal* f) {
// 	PredictionBalancer* b = new PredictionBalancer();
// 	b->f = f;
// 	return b;
// }