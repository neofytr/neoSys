#ifndef SYS_H
#define SYS_H

#define public         // these are functuons that are exported by the kernel to the user (syscall API)
#define private static // static means only for that translation unit; these are functions used internally in the kernel

// this file contains the headers for the system calls in the operating system neoSys

#include <stdbool.h>
#include <stdint.h> // we will compile using -m16 for production since we are building
                    // a 16-bit OS for a 16-bit processor.
                    // so, we will get the correct int sizes for that machine when using standard C integers

typedef uint16_t fd_t;
typedef uint16_t err_t;

err_t errno;

// our OS api functions will return zero on error and 1 on success
// the global variable errno will be set to indicate the error in the most recent
// failed call to the system

// making this variable global results in making the entire OS single-threaded

#define reterr(x)    \
    do               \
    {                \
        errno = (x); \
        return 0;    \
    } while (false)

// here are some error codes and their meaning

#define ErrBadFD (8) // the file descriptor onto which the function is being called is not associated
                     // with an open file

uint8_t load(fd_t file);               // read one byte from the file descriptor file_desc
uint8_t store(fd_t file, uint8_t chr); // store the byte data into the file descriptor file_desc

#endif