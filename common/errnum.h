#ifndef ERRNUM_H
#define ERRNUM_H

#include <stdint.h>
#include <base.h>

typedef uint16_t err_t;

#ifdef INSIDE_SYS
public
err_t errnum; // there should only be one copy of this variable, and that should
              // be inside the OS, in sys.c
              // rest all files should refer to that copy only
#else
extern err_t errnum;
#endif

// our OS api functions will return zero on error and 1 on success
// the global variable errno will be set to indicate the error in the most recent
// failed call to the system

// making this variable global results in making the entire OS single-threaded

#define ret_err(x)    \
    do                \
    {                 \
        errnum = (x); \
        return false; \
    } while (false)

#define ret_success       \
    do                    \
    {                     \
        errnum = SUCCESS; \
        return true;      \
    } while (false)

// here are some error codes and their meaning

#define SUCCESS (0)

#define ErrBadFD (8) // the file descriptor onto which the function is being called is not associated
                     // with an open file
#define ErrIO (9)    // unspecified system IO failure

#endif