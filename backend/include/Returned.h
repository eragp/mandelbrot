typedef struct {
    int x,y, zoom, size;
    int n[256][256];  // TODO Hardcoded, possible to make flexible?
    int world_rank;
} Returned;