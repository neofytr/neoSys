#include <filesys.h>
#include <stdint.h>
#include <base.h>
#include <osapi.h>
#include <disk.h>

// each block is 512 bytes

#define MAGIC1 (0xdd05)
#define MAGIC2 (0xaa55)
#define INODES_PER_BLOCK (16) // in each block, there will be 16 inodes, that are 32-byte each
#define PTR_PER_INODE (8)     // in each inode, there will be 8, 16-bit direct pointers to data blocks
#define PTR_PER_BLOCK (256)   // there will be an indirect ptr in the inode that will point to a block of 256, 2-byte data block pointers

// data blocks are the ones acutally holding the data

typedef internal struct
{
    uint8_t drive_num;     // the drive number
    drive_t *drive;        // the drive descriptor
    bool *bitmap;          // free block bitmap
    superblock_t metadata; // the superblock
} filesys_t;