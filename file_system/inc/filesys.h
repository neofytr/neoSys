#ifndef FILESYS_H
#define FILESYS_H

#include <stdint.h>
#include <base.h>
#include <disk.h>

/*
 * simple filesystem structure overview:
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

/*
 * superblock: the first block (block 0) of the filesystem
 * contains all metadata needed to understand the filesystem layout
 */
typedef internal struct
{
    uint8_t boot_sector[BOOT_SECTOR_SIZE]; // space for bootloader code
    uint16_t reserved;                     // padding/future use
    uint16_t blocks;                       // total blocks in filesystem
    uint16_t inode_blocks;                 // how many blocks are used for inodes
    uint16_t inodes;                       // total number of inodes available
    uint16_t magic1;                       // filesystem signature part 1
    uint16_t magic2;                       // filesystem signature part 2
} superblock_t;

/*
 * filename structure: stores file name in 8.3 format
 * separate name and extension fields for easier manipulation
 */
typedef internal struct
{
    uint8_t name[FILENAME_LEN];     // filename part (e.g., "document")
    uint8_t extension[FILEEXT_LEN]; // extension part (e.g., "txt")
} filename_t;

/*
 * inode: represents a single file or directory
 * contains all metadata and pointers to locate the file's data
 */
typedef internal struct
{
    // file status and type information packed into single byte
    packed struct
    {
        uint8_t reserved : 4; // reserved bits for future use
        uint8_t type : 3;     // file type (regular file, directory, etc.)
        bool valid : 1;       // whether this inode is in use
    } valid_type;

    uint16_t file_size;                 // file size in bytes
    filename_t file_name;               // file name and extension
    uint8_t *indirect_ptr;              // points to block containing 256 data block pointers
    uint8_t *direct_ptr[PTR_PER_INODE]; // direct pointers to first 8 data blocks
} inode_t;

/*
 * filesystem descriptor: main structure for mounted filesystem
 * contains all information needed to access files on this filesystem
 */
typedef internal struct
{
    uint8_t drive_num;     // which physical drive this filesystem is on
    drive_t *drive;        // pointer to drive hardware descriptor
    bool *bitmap;          // free/used block tracking bitmap
    superblock_t metadata; // copy of superblock for quick access
} filesys_t;

/*
 * generic data block union: represents different uses of a 512-byte block
 * allows same memory to be interpreted as different data types
 */
typedef internal union
{
    superblock_t superblock;     // when block 0 contains filesystem metadata
    uint8_t data[BLOCK_SIZE];    // when block contains raw file data
    uint8_t *ptr[PTR_PER_BLOCK]; // when block contains indirect pointers
    inode_t inode;               // when block contains inode data (though typically 16 per block)
} datablock_t;

#undef FILENAME_LEN
#undef FILEEXT_LEN
#undef BOOT_SECTOR_SIZE

#endif // FILESYS_H