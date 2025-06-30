#ifndef OSAPI_H
#define OSAPI_H

#include <base.h>
#include <syscalls.h>
#include <stdint.h>
#include <disk.h>

// this macro defines a custom printf wrapper called kprintf that always appends a newline to the output.
// it uses variadic macro syntax, which allows the macro to accept a variable number of arguments.
// in 'kprintf(f, ...)', 'f' is the first fixed argument (typically the format string like "value: %d")
// and '...' represents a variable number of additional arguments, similar to printf itself.
//
// '__VA_ARGS__' is a predefined macro that gets replaced by all the arguments passed in place of '...'.
// so for example, calling kprintf("value: %d", 42) will expand to printf("value: %d\n", 42)
//
// the '##__VA_ARGS__' syntax is a gcc extension that allows the macro to work even when no extra arguments are provided.
// normally, if no variadic arguments are passed, a trailing comma before '__VA_ARGS__' would cause a syntax error.
// the '##' operator before '__VA_ARGS__' removes the preceding comma if '__VA_ARGS__' is empty.
//
// this means the macro works both with and without extra arguments:
//   kprintf("hello")           → printf("hello\n")           // no extra arguments
//   kprintf("value: %d", 42)   → printf("value: %d\n", 42)   // one extra argument
//
// this macro is only for printf-like usage. it simplifies logging and ensures consistent line endings.
//
// note: '...' and '__VA_ARGS__' in macros are different from variadic functions,
//       which use va_list, va_start, va_arg, and va_end to access arguments at runtime.
//       variadic functions do not know the number or types of arguments unless encoded in the fixed parameters.
//       in contrast, variadic macros are expanded by the preprocessor and directly substitute the text inline.
#define kprintf(f, ...) printf(f "\n", ##__VA_ARGS__)

// returns the length of a null-terminated c string
internal uint16_t stringlen(void *str);

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