#ifndef OSAPI_H
#define OSAPI_H

#include <base.h>
#include <sys.h>
#include <stdint.h>

#define MAX_FD (1U << 8)

#ifdef INSIDE_SYS

internal int file_desc_arr[MAX_FD] = {
    0,
    1,
    2,
    -1,
}; // file_desc_arr[file] is the posix fd mapped to the neoSys fd file
   // it's -1 if  there is no mapping

#else

extern int *file_desc_arr;

#endif

// the neoSys fd 0, 1, and 2 are the same as the posix fd 0, 1, and 2 respectively
// neoSys fd 0 -> stdin
// neoSys fd 1 -> stdout
// neoSys fd 2 -> stderr

#define get_posix_fd(x) (file_desc_arr[(x)])

internal uint16_t stringlen(void *str);                       // length of a traditional null-terminated C-string
internal bool isopen(const fd_t file);                        // check if the file desc file points to an open file
internal bool zero(void *mem, uint16_t bytes);                // zeroes bytes number of bytes, starting at mem
internal bool copy(void *dest, void *source, uint16_t bytes); // copies bytes number of bytes from source to destination; correctly handles overlapping regions
internal uint8_t *strnum(uint8_t *str, uint8_t num);          // returns a pointer to a static piece of memory (static string) that is the string str with the number num in decimal appended to it
                                                              // the maximum length of the final string is 256; so, overflow can occur if the binary rep of the number makes the string longer than 256, in which case, the
                                                              // requisite lower digits of num will be left out in the resultant string
#endif