#ifndef SYS_H
#define SYS_H

#include <stdint.h> // we will compile using -m16 for production since we are building
                    // a 16-bit OS for a 16-bit processor.
                    // so, we will get the correct int sizes for that machine when using standard C integers

typedef uint16_t fd_t;

// this file contains the headers for the system calls in the operating system neoSys
uint8_t load(fd_t file);               // read one byte from the file descriptor file_desc
uint8_t store(fd_t file, uint8_t chr); // store the byte data into the file descriptor file_desc

#endif