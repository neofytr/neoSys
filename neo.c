#include "buildsysdep/neobuild.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define SRC "src/"
#define SHELL "shell/"
#define SYS "syscalls/"
#define DISK "disk_emulator/"
#define OSAPI "osapi/"
#define FILESYS "file_system/"
#define LIB "lib/"
#define NEOSTD "neostd/"
#define UTILS "utils/"
#define DISKUTIL "diskutil/"
#define COMMON "common/"
#define INC "inc/"
#define BIN "bin/"

#ifdef DEBUG
#define DEBUG_FLAGS "-g3"
#else
#define DEBUG_FLAGS ""
#endif

#define SO_FLAGS "-ldl -shared" // create a shared library, with support for dynamic loading

#define CHECK_AND_RETURN(ret) \
    if (!ret)                 \
        return EXIT_FAILURE;

#define DEBUG
#define CFLAGS "-O2 -Wall -fPIC"     \
               " -I " SYS INC        \
               " -I " SHELL INC      \
               " -I " OSAPI INC      \
               " -I " DISK INC       \
               " -I " FILESYS INC    \
               " -I " COMMON         \
               " -I " LIB NEOSTD INC \
                   DEBUG_FLAGS

#define LFLAGS NULL

int main(int argc, char **argv)
{
    bool ret;
    int retval;
    neorebuild("neo.c", argv, &argc);

    if (argc > 1 && !strcmp(argv[1], "clean"))
    {
        neocmd_t *rm = neocmd_create(BASH);

        neocmd_append(rm, "rm -rf");
        neocmd_append(rm, DISK SRC "disk.o");
        neocmd_append(rm, SHELL SRC "shell.o");
        neocmd_append(rm, SYS SRC "syscalls.o");
        neocmd_append(rm, OSAPI SRC "osapi.o");
        neocmd_append(rm, BIN "libos.so shell.neo");
        neocmd_append(rm, UTILS DISKUTIL SRC "diskutil.o");
        neocmd_append(rm, UTILS DISKUTIL BIN "diskutil.neo");

        neocmd_run_sync(rm, NULL, NULL, false);
        neocmd_delete(rm);
        return EXIT_SUCCESS;
    }

    ret = neo_compile_to_object_file(GCC, SHELL SRC "shell.c", NULL, CFLAGS, false);
    CHECK_AND_RETURN(ret);

    ret = neo_compile_to_object_file(GCC, SYS SRC "syscalls.c", NULL, CFLAGS, false);
    CHECK_AND_RETURN(ret);

    ret = neo_compile_to_object_file(GCC, LIB NEOSTD SRC "neostd.c", NULL, CFLAGS, false);
    CHECK_AND_RETURN(ret);

    ret = neo_compile_to_object_file(GCC, DISK SRC "disk.c", NULL, CFLAGS, false);
    CHECK_AND_RETURN(ret);

    ret = neo_compile_to_object_file(GCC, OSAPI SRC "osapi.c", NULL, CFLAGS, false);
    CHECK_AND_RETURN(ret);

    ret = neo_compile_to_object_file(GCC, FILESYS SRC "filesys.c", NULL, CFLAGS, false);
    CHECK_AND_RETURN(ret);

    // now we make all the kernel stuff into a shared library
    neocmd_t *cmd = neocmd_create(BASH);
    CHECK_AND_RETURN(cmd);

    neocmd_append(cmd, "gcc", "-o bin/libos.so");
    neocmd_append(cmd, SO_FLAGS);
    neocmd_append(cmd, SYS SRC "syscalls.o");
    neocmd_append(cmd, DISK SRC "disk.o");
    neocmd_append(cmd, OSAPI SRC "osapi.o");
    neocmd_append(cmd, FILESYS SRC "filesys.o");

    neocmd_run_sync(cmd, NULL, NULL, false);
    neocmd_delete(cmd);

    neo_link(GCC, BIN "shell.neo", "-L " BIN " -los"
                                   " -Wl,-rpath=./bin",
             false, SHELL SRC "shell.o", LIB NEOSTD SRC "neostd.o");

    // compiling and linking the disk utility
    neo_compile_to_object_file(GCC, UTILS DISKUTIL SRC "diskutil.c", NULL, CFLAGS, false);

    // the disk utility needs some internal kernel headers and functions to link with it
    // so that it can do it's work properly
    neo_link(GCC, UTILS DISKUTIL BIN "diskutil.neo", NULL, false, UTILS DISKUTIL SRC "diskutil.o", OSAPI SRC "osapi.o", DISK SRC "disk.o", FILESYS SRC "filesys.o");
    return EXIT_SUCCESS;
}