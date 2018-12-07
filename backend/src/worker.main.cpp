#include "init.h"

#ifdef _OPENMP
    #include <omp.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <iostream>
#endif

int main(int argc, char *argv[])
{
    #ifdef _OPENMP
        if (!omp_get_cancellation()) {
            std::cout << "Restarting to activate OpenMP Cancellation" << std::endl;
            putenv((char *) "OMP_CANCELLATION=true");
            execv(argv[0], argv);
        }
    #endif
    
    return init(argc, argv, false);
}
