#ifndef SYS_H
#define SYS_H

#include <base.h>

// this file contains the headers for the system calls in the operating system neoSys
#include <stdint.h> // we will compile using -m16 for production since we are building
                    // a 16-bit OS for a 16-bit processor.
                    // so, we will get the correct int sizes for that machine when using standard C integers
#include <stdbool.h>

typedef uint8_t fd_t;

public
uint8_t load(const fd_t file); // read one byte from the file descriptor file_desc
public
bool store(const fd_t file, const uint8_t chr); // store the byte data into the file descriptor file_desc

#endif