#include <filesys.h>
#include <osapi.h>
#include <errnum.h>
#include <stdlib.h>

private uint16_t find_free_block(bitmap_t bitmap, uint16_t total_blocks);
private bool mark_block_used(bitmap_t bitmap, uint16_t block_num);
private void mark_block_free(bitmap_t bitmap, uint16_t block_num);

// bitmap helper functions
internal void set_bit(bitmap_t bitmap, uint16_t block_num)
{
    bitmap[block_num / 8] |= (1 << (block_num % 8));
}

internal void clear_bit(bitmap_t bitmap, uint16_t block_num)
{
    bitmap[block_num / 8] &= ~(1 << (block_num % 8));
}

internal bool get_bit(bitmap_t bitmap, uint16_t block_num)
{
    return (bitmap[block_num / 8] & (1 << (block_num % 8))) != 0;
}

internal bitmap_t mkbitmap(filesys_t *filesys, bool scan)
{
    if (!filesys)
        return NULL;

    // calculate bitmap size in bytes (1 bit per block, rounded up)
    uint16_t size = (filesys->drive->blocks + 7) / 8;

    // allocate and zero bitmap
    bitmap_t bitmap = malloc(size);
    if (!bitmap)
        return NULL;
    zero(bitmap, size);

    if (!scan)
        return bitmap;

    // mark superblock and all inode blocks as used
    for (uint16_t blk = 0; blk <= filesys->super_block.inode_blocks; blk++)
    {
        set_bit(bitmap, blk);
    }

    // scan all inodes to mark used data blocks
    uint8_t buf[BLOCK_SIZE];
    for (uint16_t blk = 1; blk <= filesys->super_block.inode_blocks; blk++)
    {
        if (!d_read(filesys->drive, buf, blk))
        {
            free(bitmap);
            return NULL;
        }

        // check each inode in this block
        for (uint16_t node = 0; node < INODES_PER_BLOCK; node++)
        {
            inode_t *inode = (inode_t *)buf + node;
            if (inode->file_type == TYPE_NOT_VALID)
                continue;

            // mark direct pointers as used (assuming they store block numbers directly)
            for (uint16_t ptr = 0; ptr < PTR_PER_INODE; ptr++)
            {
                uint16_t blocknum = (uint16_t)(uintptr_t)inode->direct_ptr[ptr];
                if (blocknum)
                {
                    set_bit(bitmap, blocknum);
                }
            }

            // mark indirect block and its pointers as used
            uint16_t indirect_ptr = (uint16_t)(uintptr_t)inode->indirect_ptr;
            if (indirect_ptr)
            {
                set_bit(bitmap, indirect_ptr);

                // read indirect block and mark all referenced blocks
                uint8_t indirect_buf[BLOCK_SIZE];
                if (!d_read(filesys->drive, indirect_buf, indirect_ptr))
                {
                    free(bitmap);
                    return NULL;
                }

                // parse indirect block as array of block numbers
                uint16_t *block_ptrs = (uint16_t *)indirect_buf;
                for (uint16_t ptr = 0; ptr < PTR_PER_BLOCK; ptr++)
                {
                    uint16_t blocknum = block_ptrs[ptr];
                    if (blocknum)
                    {
                        set_bit(bitmap, blocknum);
                    }
                }
            }
        }
    }

    return bitmap;
}

private uint16_t find_free_block(bitmap_t bitmap, uint16_t total_blocks)
{
    // find first free block after reserved area
    for (uint16_t i = 0; i < total_blocks; i++)
    {
        if (!get_bit(bitmap, i))
            return i;
    }
    return 0; // no free blocks
}

private bool mark_block_used(bitmap_t bitmap, uint16_t block_num)
{
    if (get_bit(bitmap, block_num))
        return false; // already used
    set_bit(bitmap, block_num);
    return true;
}

private void mark_block_free(bitmap_t bitmap, uint16_t block_num)
{
    clear_bit(bitmap, block_num);
}

internal filesys_t *fs_format(drive_t *drive, bootsec_t *boot_sector, bool force)
{
    if (!drive)
        return NULL;

    // check for open files, erase if forced
    if (openfiles(drive))
    {
        if (!force)
            return NULL;
        erase_all_files(drive);
    }

    // allocate filesystem structure
    filesys_t *filesys = malloc(sizeof(filesys_t));
    if (!filesys)
        return NULL;

    // calculate inode blocks (10% of total, rounded up)
    uint16_t inode_blocks = (drive->blocks + 9) / 10;

    // initialize superblock
    filesys->super_block.magic1 = MAGIC1;
    filesys->super_block.magic2 = MAGIC2;
    filesys->super_block.inodes = 1;
    filesys->super_block.blocks = drive->blocks;
    filesys->super_block.inode_blocks = inode_blocks;
    filesys->super_block.reserved = 0;

    // handle boot sector
    if (boot_sector)
    {
        copy(&filesys->super_block.boot_sector, boot_sector, BOOT_SECTOR_SIZE);
    }
    else
    {
        zero(&filesys->super_block.boot_sector, BOOT_SECTOR_SIZE);
    }

    // set drive info
    filesys->drive = drive;
    filesys->drive_num = drive->drive_num;

    // write superblock to drive
    if (!d_write(drive, (uint8_t *)&filesys->super_block, 0))
    {
        free(filesys->bitmap);
        free(filesys);
        return NULL;
    }

    // prepare root directory inode
    inode_t root_inode;
    zero((void *)&root_inode, sizeof(root_inode));
    root_inode.file_type = TYPE_DIR;

    // write root inode to first inode block
    uint8_t buf[BLOCK_SIZE];
    zero(buf, BLOCK_SIZE);
    copy(buf, &root_inode, sizeof(root_inode));

    if (!d_write(drive, buf, 1))
    {
        free(filesys->bitmap);
        free(filesys);
        return NULL;
    }

    // zero remaining inode blocks
    zero(buf, BLOCK_SIZE);
    for (uint16_t i = 2; i <= inode_blocks; i++)
    {
        if (!d_write(drive, buf, i))
        {
            free(filesys->bitmap);
            free(filesys);
            return NULL;
        }
    }

    // create initial bitmap
    filesys->bitmap = mkbitmap(filesys, true);
    if (!filesys->bitmap)
    {
        free(filesys);
        return NULL;
    }

    return filesys;
}