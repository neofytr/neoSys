#ifndef FILESYS_H
#define FILESYS_H

#include <stdint.h>
#include <base.h>
#include <disk.h>

/*
 * filesystem structure overview:
 *
 * block 0: superblock (contains metadata about the entire filesystem)
 * block 1 to n: inode blocks (each block contains 16 inodes, 32 bytes each)
 * block n+1 onwards: data blocks (actual file content and indirect pointer blocks)
 *
 * each file is represented by an inode that contains:
 * - file metadata (name, size, type)
 * - 8 direct pointers to data blocks (for small files)
 * - 1 indirect pointer to a block containing 256 more data block pointers (for large files)
 *
 * this allows files up to: (8 + 256) * 512 = 135,168 bytes (~132kb) per file
 */

/*

Block 1 to n are inode blocks.
They are 10% (rounded up) of the total blocks on the drive.
Each inode block contains 16, 32-byte inodes, indexed from 0 to 31 (inode_index_in_block)
The inode index of an inode is given by ((inode_block_index - 1) * INODES_PER_BLOCK) + inode_index_in_block
The inode block index of an inode is given by (inode_index / INODES_PER_BLOCK) + 1
All indexes START from 0.

*/

// bitmap macros
#define set_bit(bitmap, blk)                              \
    do                                                    \
    {                                                     \
        if ((bitmap))                                     \
            (bitmap)[(blk) >> 3U] |= (1U << ((blk) & 7)); \
    } while (0)
#define clear_bit(bitmap, blk)                             \
    do                                                     \
    {                                                      \
        if ((bitmap))                                      \
            (bitmap)[(blk) >> 3U] &= ~(1U << ((blk) & 7)); \
    } while (0)
#define get_bit(bitmap, blk) ((bitmap)[(blk) >> 3U] & (1U << ((blk) & 7)))

// filesystem constants
#define FILENAME_LEN (8)       // maximum filename length (8.3 format)
#define FILEEXT_LEN (3)        // maximum file extension length
#define BOOT_SECTOR_SIZE (500) // boot code area size in superblock

// magic numbers for filesystem validation
#define MAGIC1 (0xdd05) // first magic number to identify valid filesystem
#define MAGIC2 (0xaa55) // second magic number (common boot signature)

// layout constants
#define INODES_PER_BLOCK (16) // how many 32-byte inodes fit in each 512-byte block
#define PTR_PER_INODE (8)     // direct data block pointers per inode
#define PTR_PER_BLOCK (256)   // indirect pointers per block (512 bytes / 2 bytes per pointer)

// bootsec_t is an alias for the type uint8_t[BOOT_SECTOR_SIZE], i.e, an array of BOOT_SECTOR_SIZE bytes
typedef uint8_t bootsec_t[BOOT_SECTOR_SIZE];

/*
 * superblock: the first block (block 0) of the filesystem
 * contains all metadata needed to understand the filesystem layout
 */
typedef struct packed
{
    bootsec_t boot_sector; // space for bootloader code
    uint16_t reserved;     // padding/future use
    uint16_t blocks;       // total blocks in filesystem
    uint16_t inode_blocks; // how many blocks are used for inodes
    uint16_t inodes;       // total number of inodes currently used (and NOT the total possible number of inodes)
    uint16_t magic1;       // filesystem signature part 1
    uint16_t magic2;       // filesystem signature part 2
} superblock_t;            // packed ensures that this structure is always 512 bytes

/*
 * filename structure: stores file name in 8.3 format
 * separate name and extension fields for easier manipulation
 */
typedef struct packed
{
    uint8_t name[FILENAME_LEN];     // filename part (e.g., "document")
    uint8_t extension[FILEEXT_LEN]; // extension part (e.g., "txt")
} filename_t;                       // packed ensures this structure is always 11 bytes

/* typedef enum
{
    TYPE_NOT_VALID = 0x00,
    TYPE_FILE = 0x01,
    TYPE_DIR = 0x03,
} filetype_t;
 */

// can't have filetype_t
#define TYPE_NOT_VALID 0x00
#define TYPE_FILE 0x01
#define TYPE_DIR 0x03

/*
 * inode: represents a single file or directory
 * contains all metadata and pointers to locate the file's data
 */
typedef struct packed
{
    // file status and type information packed into single byte
    uint8_t file_type;

    uint16_t file_size;                 // file size in bytes
    filename_t file_name;               // file name and extension
    uint16_t indirect_ptr;              // block number of a block containing 256 data block numbers
    uint16_t direct_ptr[PTR_PER_INODE]; // block numbers of the first 8 data blocks
} inode_t;                              // packed ensures this structure is always 32 bytes

typedef uint8_t *bitmap_t; // any bitmap_t variable is passed as a reference by defaul

/*
 * filesystem descriptor: main structure for mounted filesystem
 * contains all information needed to access files on this filesystem
 */
typedef struct packed
{
    uint8_t drive_num;        // which physical drive this filesystem is on
    drive_t *drive;           // pointer to drive hardware descriptor
    bitmap_t bitmap;          // free/used block tracking bitmap (bit r of bitmap is linked to block r; 0 <= r < block_num)
    superblock_t super_block; // copy of superblock for quick access
} filesys_t;

/*
 * generic data block union: represents different uses of a 512-byte block
 * allows same memory to be interpreted as different data types
 */
typedef union
{
    superblock_t superblock;         // when block 0 contains filesystem metadata
    uint8_t data[BLOCK_SIZE];        // when block contains raw file data
    uint16_t ptr[PTR_PER_BLOCK];     // when block contains indirect pointers
    inode_t inode[INODES_PER_BLOCK]; // when block contains inode data (though typically 16 per block)
} datablock_t;                       // this data type is always BLOCK_SIZE (512 bytes)

public
void filesys_test(drive_t *drive);

internal bitmap_t fs_mkbitmap(filesys_t *filesys, bool scan); // returns NULL upon failure
internal void fs_dltbitmap(bitmap_t bitmap);                  // destroys bitmap
internal filesys_t *fs_format(drive_t *drive, bootsec_t *boot_sector, bool force);
internal uint16_t fs_first_free(filesys_t *filesys);                                  // returns the blocknum of the first free block in the filesystem; returns 0 on error
internal void fs_show(filesys_t *filesys, bool show_bitmap);                          // prints filesystem metadata
internal bool fs_get_inode(filesys_t *filesys, uint16_t inode_index, inode_t *inode); // inode index starts from 0; returns false if inode_index is out of range; gets the inode with index inode_index

#endif // FILESYS_H