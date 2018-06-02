#ifndef TILEINFO_H	// Prevents multiple includes of this header
#define TILEINFO_H

struct TileInfo {
    double minReal; // included
    double maxReal; // excluded
    double minImaginary; // included
    double maxImaginary; // excluded

	// Resolution
	int xRes; // width
	int yRes; // height
};

#endif // !TILEINFO_H
