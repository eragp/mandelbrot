#include "Region.h"

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
     * The time that computation has taken in nanoseconds
     */
    long long computationTime;

    /**
     * The region that was computed.
     */
    Region region;
    
};