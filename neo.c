#include "buildsysdep/neobuild.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define SRC "src/"
#define SHELL "shell/"
#define SYS "syscalls/"
#define EMU "disk_emulator/"
#define COMMON "common/"
#define INC "inc/"
#define BIN "bin/"

#define SO_FLAGS "-ldl -fPIC -shared" // create a shared library, with support for dynamic loading

#define CFLAGS "-O2 -Wall "     \
               "-I " SYS INC    \
               " -I " SHELL INC \
               " -I " COMMON
#define LFLAGS NULL

int main(int argc, char **argv)
{
    bool ret;
    int retval;
    neorebuild("neo.c", argv, &argc);

    if (argc > 1 && !strcmp(argv[1], "clean"))
    {
        retval = system("rm " BIN "shell.neo " SHELL SRC "shell.o " SYS SRC "sys.o " BIN "osapi.so");
        if (retval < 0)
        {
            fprintf(stderr, "CLEAN FAILED!\n");
            fprintf(stderr, "ERROR -> %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    ret = neo_compile_to_object_file(GCC, SHELL SRC "shell.c", NULL, CFLAGS, false);
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

    ret = neo_compile_to_object_file(GCC, EMU SRC "emu.c", NULL, CFLAGS, false);
    if (!ret)
    {
        return EXIT_FAILURE;
    }

    // now we make it a shared library
    neocmd_t cmd;

    neocmd_append(&cmd, "gcc", "-o bin/osapi.so");
    neocmd_append(&cmd, SO_FLAGS);
    neocmd_append(&cmd, "sys.o emu.o");

    return EXIT_SUCCESS;
}