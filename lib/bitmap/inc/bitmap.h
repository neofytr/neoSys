
#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <stdint.h>

#define set_bit(bitmap, blk)                                         \
    do                                                               \
    {                                                                \
        if ((bitmap) && (bitmap)->ptr && blk < (bitmap)->total_bits) \
            (bitmap)->ptr[(blk) >> 3U] |= (1U << ((blk) & 7));       \
    } while (0)
#define clear_bit(bitmap, blk)                                       \
    do                                                               \
    {                                                                \
        if ((bitmap) && (bitmap)->ptr && blk < (bitmap)->total_bits) \
            (bitmap)->ptr[(blk) >> 3U] &= ~(1U << ((blk) & 7));      \
    } while (0)
#define get_bit(bitmap, blk) ((bitmap)->ptr[(blk) >> 3U] & (1U << ((blk) & 7)))

typedef struct
{
    uint8_t *ptr;
    uint16_t total_bits;
} bitmap_t;

public
bitmap_t bitmap_create(uint16_t bits); // returns NULL on error

public
void bitmap_destroy(bitmap_t bitmap); // destroys a bitmap

#endif