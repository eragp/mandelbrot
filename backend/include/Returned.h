typedef struct {
    int start_x;  //included
    int start_y;  //included
    int end_x;    //excluded
    int end_y;    //excluded
    int z;
    int size;
    int n[256][256];  // TODO Hardcoded, possible to make flexible?
    int world_rank;
} Returned;