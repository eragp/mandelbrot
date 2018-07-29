#pragma once

#include "Region.h"

// @Tobi: What is this for? Included it because you had it in your branch
#pragma pack(push, 1)

/**
 * Information sent by Worker when done with computing data.
 * Contains constant amount of metadata.
 */
struct WorkerInfo {

    /**
     * The rank of the worker, that has computed the following data
     */
    int rank;

    /**
     * The time that computation has taken in microseconds
     */
    unsigned long computationTime;

    /**
     * The region that was computed.
     */
    Region region;
    
};

// @Tobi: Same as above
#pragma pack(pop)