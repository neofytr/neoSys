#ifndef SYS_H
#define SYS_H

#include <base.h>

// lower areas of memory will be where the OS will reside
// these are outside of access rights of user space

// this file contains the headers for the system calls in the operating system neoSys
#include <stdint.h> // we will compile using -m16 for production since we are building
                    // a 16-bit OS for a 16-bit processor.
                    // so, we will get the correct int sizes for that machine when using standard C integers
#include <stdbool.h>

typedef uint8_t fd_t;

// maximum number of file descriptors supported by neosys
#define MAX_FD (1U << 8)

// if we are inside of the kernel, we can use the file_desc_arr
// if we are inside of the user space, we can't use it
// since it is outside of the access rights of the user space
// this array is defined only once, inside the sys.c file (kernel)
#ifdef INSIDE_SYS
// maps neosys file descriptors to posix file descriptors
// file_desc_arr[file] holds the posix fd mapped to neosys fd 'file'
// -1 indicates no mapping exists for that neosys fd
internal int file_desc_arr[MAX_FD] = {
    0,  // neosys fd 0 -> posix stdin
    1,  // neosys fd 1 -> posix stdout
    2,  // neosys fd 2 -> posix stderr
    -1, // remaining slots unmapped
};
#else
#ifdef INSIDE_KERN // the rest of the kernel can reference file_desc_arr when being linked together with sys.o
                   // but the user space can't
                   // internal visibility makes sure that this variable isn't exported in the final executable
extern int *file_desc_arr;
#endif             // INSIDE_KERN
#endif             // INSIDE_SYS

// standard file descriptor mappings (same as posix):
// neosys fd 0 -> stdin  (standard input)
// neosys fd 1 -> stdout (standard output)
// neosys fd 2 -> stderr (standard error)

public
uint8_t load(const fd_t file); // read one byte from the file descriptor file_desc
public
bool store(const fd_t file, const uint8_t chr); // store the byte data into the file descriptor file_desc
public
bool isopen(const fd_t file); // checks if the neoSys file descriptor file is associated with some open file
                              // fails with errnum set to ErrBadFD if not
                              // succeeds otherwise

#endif