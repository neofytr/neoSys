#include "buildsysdep/neobuild.h"
#include <unistd.h>
#include <stdio.h>

#define SRC "src/"
#define BIN "bin/"
#define INC "inc/"

int main(int argc, char **argv)
{
    neorebuild("neo.c", argv);
    neo_compile_to_object_file(GCC, SRC "main.c", BIN "main.o", NULL, false);
    neo_link(GCC, BIN "main", NULL, false, BIN "main.o");
    int ret = system("rm " BIN "main.o");
    if (ret < 0)
    {
        NEO_LOG(1, "removal of main.o failed");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}