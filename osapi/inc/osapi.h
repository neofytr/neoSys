#ifndef OSAPI_H
#define OSAPI_H

#include <base.h>
#include <syscalls.h>
#include <stdint.h>
#include <disk.h>

// returns the length of a null-terminated c string
uint16_t internal stringlen(void *str);

// zeroes 'bytes' number of bytes starting at memory address 'mem'
bool internal zero(void *mem, uint16_t bytes);

// copies 'bytes' from 'source' to 'dest' memory locations
// handles overlapping memory regions correctly
bool internal copy(void *dest, void *source, uint16_t bytes);

// appends the decimal representation of 'num' to string 'str'
// returns pointer to a static buffer containing the result
// maximum result length is 256 characters
// if the final string would exceed 256 chars, the lower digits
// of 'num' are truncated to fit within the limit
uint8_t *internal strnum(uint8_t *str, uint8_t num);

uint16_t internal openfiles(drive_t *drive);

void internal erase_all_files(drive_t *drive);

#endif