#include <filesys.h>
#include <osapi.h>
#include <errnum.h>
#include <stdlib.h>

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
    uint16_t size;
    uint16_t blocks;
    uint16_t inode_blocks;
    drive_t *drive;
    bitmap_t bitmap;
    superblock_t superblock;
    uint8_t buf[BLOCK_SIZE];
    uint8_t indirect_buf[BLOCK_SIZE];
    inode_t *inode;
    uint16_t blocknum;
    uint16_t indirect_ptr;

    if (!filesys)
        return NULL;

    drive = filesys->drive;
    blocks = drive->blocks;
    size = (blocks + 7) / 8;

    bitmap = malloc(sizeof(uint8_t) * size);
    if (!bitmap)
        return NULL;

    zero((void *)bitmap, sizeof(uint8_t) * size);
    if (!scan)
        return bitmap;

    // mark superblock and all inode blocks as used
    superblock = filesys->super_block;
    inode_blocks = superblock.inode_blocks;
    for (uint16_t blk = 0; blk <= inode_blocks; blk++)
        set_bit(bitmap, blk);

    // for each valid inode, follow it's direct and indirect pointers
    zero((void *)buf, BLOCK_SIZE);
    for (uint16_t blk = 1; blk <= inode_blocks; blk++)
    {
        if (!d_read(drive, buf, blk))
        {
            free(bitmap);
            return NULL;
        }

        for (uint16_t node = 0; node < INODES_PER_BLOCK; node++)
        {
            inode = ((inode_t *)buf + node);
            if (inode->file_type == TYPE_NOT_VALID)
                continue;

            // mark direct pointers as used
            for (uint16_t ptr = 0; ptr < PTR_PER_INODE; ptr++)
            {
                blocknum = inode->direct_ptr[ptr];
                if (blocknum) // if it's zero, then it means, it's uninitialized
                    set_bit(bitmap, inode->direct_ptr[ptr]);
            }

            // mark indirect pointers as used
            indirect_ptr = inode->indirect_ptr;
            if (indirect_ptr)
            {
                set_bit(bitmap, indirect_ptr);

                // read the indirect block and mark all references
                if (!d_read(drive, indirect_buf, indirect_ptr))
                {
                    free(bitmap);
                    return NULL;
                }

                for (uint16_t ptr = 0; ptr < PTR_PER_BLOCK; ptr++)
                {
                    blocknum = *(uint16_t *)indirect_buf;
                    indirect_buf += 2;
                    if (blocknum)
                        set_bit(bitmap, blocknum);
                }
            }
        }
    }

    return bitmap;
}

internal uint16_t find_free_block(bitmap_t bitmap, uint16_t total_blocks)
{
    // find first free block after reserved area
    for (uint16_t i = 0; i < total_blocks; i++)
    {
        if (!get_bit(bitmap, i))
            return i;
    }
    return 0; // no free blocks
}

internal bool mark_block_used(bitmap_t bitmap, uint16_t block_num)
{
    if (get_bit(bitmap, block_num))
        return false; // already used
    set_bit(bitmap, block_num);
    return true;
}

internal void mark_block_free(bitmap_t bitmap, uint16_t block_num)
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

    // create initial bitmap (no scanning needed for fresh format)
    filesys->bitmap = mkbitmap(filesys, false);
    if (!filesys->bitmap)
    {
        free(filesys);
        return NULL;
    }

    // mark system blocks as used
    for (uint16_t i = 0; i <= inode_blocks; i++)
    {
        set_bit(filesys->bitmap, i);
    }

    // write superblock to drive
    if (!d_write(drive, &filesys->super_block, 0))
    {
        free(filesys->bitmap);
        free(filesys);
        return NULL;
    }

    // prepare root directory inode
    inode_t root_inode = {
        .file_type = TYPE_DIR,
        .file_size = 0,
        .indirect_ptr = NULL,
        .direct_ptr = {0},
        .file_name = {0}};

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

    return filesys;
}