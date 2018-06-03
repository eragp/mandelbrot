#ifndef TILEDATA_H	// Prevents multiple includes of this header
#define TILEDATA_H

struct TileData {
	double minReal; // included
    double maxReal; // excluded
    double minImaginary; // included
    double maxImaginary; // excluded

	// Resolution
	int xRes; // width
	int yRes; // height
	
	// Result
	int *n;
	
	// World-Rank of Node
	int world_rank;
	
	TileData(double MinReal, double MaxReal, double MinImaginary, double MaxImaginary, int XRes, int YRes, int World_rank) {
		minReal = MinReal;
		maxReal = MaxReal;
		minImaginary = MinImaginary;
		maxImaginary = MaxImaginary;
		xRes = XRes;
		yRes = YRes;
		n = new int[XRes*YRes];
		world_rank = World_rank;
	}
	
	TileData(int dimension) {
		n = new int[dimension];
	}
};

#endif // !TILEDATA_H