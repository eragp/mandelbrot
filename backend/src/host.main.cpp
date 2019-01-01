#include "init.h"
#include "Host.h"

int main(int argc, char *argv[])
{
    return init(argc, argv, "Host", Host::init);
}
