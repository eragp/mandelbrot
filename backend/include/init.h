/**
 * Initialize MPI and the current worker
 * Arguments:
 *  argc, argv: passing through arguments of the main function
 *  type: String, describing the type of worker (i.e. "host"). Used for logging output
 *  initFunc: Method for initializing the worker, accepting world_rank and world_size
 */
int init(int argc, char **argv, const char* type, void (*initFunc) (int world_rank, int world_size));