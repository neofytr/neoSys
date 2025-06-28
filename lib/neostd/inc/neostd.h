#include <base.h>
#include <errnum.h>
#include <stdint.h>
#include <stdlib.h>

#define allocate(n) malloc(n)
#define destroy(ptr) free((void *)ptr)