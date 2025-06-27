#ifndef OSAPI_H
#define OSAPI_H

#include <base.h>
#include <sys.h>
#include <stdint.h>
#include <disk.h>

// maximum number of file descriptors supported by neosys
#define MAX_FD (1U << 8)

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

extern int *file_desc_arr;

#endif

// standard file descriptor mappings (same as posix):
// neosys fd 0 -> stdin  (standard input)
// neosys fd 1 -> stdout (standard output)
// neosys fd 2 -> stderr (standard error)

// macro to get the posix fd corresponding to a neosys fd
#define get_posix_fd(x) (file_desc_arr[(x)])

// returns the length of a null-terminated c string
internal uint16_t stringlen(void *str);

// checks if the given neosys file descriptor points to an open file
internal bool isopen(const fd_t file);

// zeroes 'bytes' number of bytes starting at memory address 'mem'
internal bool zero(void *mem, uint16_t bytes);

// copies 'bytes' from 'source' to 'dest' memory locations
// handles overlapping memory regions correctly
internal bool copy(void *dest, void *source, uint16_t bytes);

// appends the decimal representation of 'num' to string 'str'
// returns pointer to a static buffer containing the result
// maximum result length is 256 characters
// if the final string would exceed 256 chars, the lower digits
// of 'num' are truncated to fit within the limit
internal uint8_t *strnum(uint8_t *str, uint8_t num);

internal uint16_t openfiles(drive_t *drive);

internal void erase_all_files(drive_t *drive);

#endif