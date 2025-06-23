#include "buildsysdep/neobuild.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define SRC "src/"
#define SHELL "shell/"
#define SYS "syscalls/"
#define INC "inc/"
#define BIN "bin/"

#define CFLAGS "-O2 -Wall "  \
               "-I " SYS INC \
               " -I " SHELL INC
#define LFLAGS NULL

int main(int argc, char **argv)
{
    neorebuild("neo.c", argv);

    if (argc > 1 && !strcmp(argv[1], "clean"))
    {
        int retval = system("rm " BIN "shell.com " SHELL SRC "shell.o " SYS SRC "sys.o");
        if (retval < 0)
        {
            fprintf(stderr, "CLEAN FAILED!\n");
            fprintf(stderr, "ERROR -> %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    bool ret = neo_compile_to_object_file(GCC, SHELL SRC "shell.c", NULL, CFLAGS, false);
    if (!ret)
    {
        return EXIT_FAILURE;
    }

    ret = neo_compile_to_object_file(GCC, SYS SRC "sys.c", NULL, CFLAGS, false);
    if (!ret)
    {
        return EXIT_FAILURE;
    }

    ret = neo_link(GCC, BIN "shell.neo", LFLAGS, false, SHELL SRC "shell.o", SYS SRC "sys.o");
    if (!ret)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}