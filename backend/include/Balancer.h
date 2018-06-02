#pragma once
#ifndef BALANCER_H	// Prevents multiple includes of this header
#define BALANCER_H
#include "TileInfo.h"

class Balancer {
	public:
		virtual TileInfo* balanceLoad(TileInfo tile, int nodeCount) = 0;
};

#endif