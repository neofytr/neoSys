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
    int ret;
    neorebuild("neo.c", argv);

    if (argc > 1 && !strcmp(argv[1], "clean"))
    {
        ret = system("rm " BIN "shell.com " SHELL SRC "shell.o " SYS SRC "sys.o");
        if (ret < 0)
        {
            fprintf(stderr, "CLEAN FAILED!\n");
            fprintf(stderr, "ERROR -> %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    neo_compile_to_object_file(GCC, SHELL SRC "shell.c", NULL, CFLAGS, false);
    neo_compile_to_object_file(GCC, SYS SRC "sys.c", NULL, CFLAGS, false);

    neo_link(GCC, BIN "shell.neo", LFLAGS, false, SHELL SRC "shell.o", SYS SRC "sys.o");

    return EXIT_SUCCESS;
}