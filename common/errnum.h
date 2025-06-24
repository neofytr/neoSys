#ifndef ERRNUM_H
#define ERRNUM_H

#include <stdint.h>
#include <base.h>

typedef uint16_t err_t;

public
err_t errnum;

// our OS api functions will return zero on error and 1 on success
// the global variable errno will be set to indicate the error in the most recent
// failed call to the system

// making this variable global results in making the entire OS single-threaded

#define reterr(x)     \
    do                \
    {                 \
        errnum = (x); \
        return 0;     \
    } while (false)

// here are some error codes and their meaning

#define ErrBadFD (8) // the file descriptor onto which the function is being called is not associated
                     // with an open file

#endif