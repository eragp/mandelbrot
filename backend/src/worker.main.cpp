#include "init.h"
#include "Worker.h"

int main(int argc, char *argv[])
{
    return init(argc, argv, "Worker", Worker::init);
}
