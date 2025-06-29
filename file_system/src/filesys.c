#include <filesys.h>
#include <osapi.h>
#include <errnum.h>
#include <stdlib.h>

private uint16_t find_free_block(bitmap_t bitmap, uint16_t total_blocks);
private bool mark_block_used(bitmap_t bitmap, uint16_t block_num);
private void mark_block_free(bitmap_t bitmap, uint16_t block_num);

internal bitmap_t fs_mkbitmap(filesys_t *filesys, bool scan)
{
    uint16_t size, ptr, node, blocknum, indirect_ptr, blocks, blk, inode_blocks;
    bitmap_t bitmap;
    datablock_t buf, indirect_buf;
    drive_t *drive;
    uint16_t *block_ptr;
    inode_t *inode;

    if (!filesys)
        return NULL;

    // calculate bitmap size in bytes (1 bit per block, rounded up)
    drive = filesys->drive;
    blocks = drive->blocks;
    size = (blocks + 7) / 8;

    // allocate and zero bitmap
    bitmap = malloc(size);
    if (!bitmap)
        return NULL;

    if (!scan)
        return bitmap;

    // mark superblock and all inode blocks as used

    inode_blocks = filesys->super_block.inode_blocks;
    for (blk = 0; blk <= inode_blocks; blk++)
        set_bit(bitmap, blk);

    // if a blockptr in an inode is zero, that means it's uninitialized since
    // we set everything to zero by default;
    // also, 0 is the blockptr for the superblock

    // scan all inodes to mark used data blocks
    for (blk = 1; blk <= inode_blocks; blk++)
    {
        if (!d_read(drive, (uint8_t *)&buf, blk))
        {
            free(bitmap);
            return NULL;
        }

        // check each inode in this block
        for (node = 0; node < INODES_PER_BLOCK; node++)
        {
            inode = &(buf.inode[node]);
            if (inode->file_type == TYPE_NOT_VALID)
                continue;

            // mark direct pointers as used (assuming they store block numbers directly)
            for (ptr = 0; ptr < PTR_PER_INODE; ptr++)
            {
                blocknum = (uint16_t)inode->direct_ptr[ptr];
                if (blocknum)
                    set_bit(bitmap, blocknum);
            }

            // mark indirect block and its pointers as used
            indirect_ptr = (uint16_t)inode->indirect_ptr;
            if (indirect_ptr)
            {
                set_bit(bitmap, indirect_ptr);

                // read indirect block and mark all referenced blocks
                if (!d_read(drive, (uint8_t *)indirect_buf.data, indirect_ptr))
                {
                    free(bitmap);
                    return NULL;
                }

                // parse indirect block as array of block numbers
                block_ptr = (uint16_t *)indirect_buf.ptr;
                for (ptr = 0; ptr < PTR_PER_BLOCK; ptr++)
                {
                    blocknum = block_ptr[ptr];
                    if (blocknum)
                        set_bit(bitmap, blocknum);
                }
            }
        }
    }

    return bitmap;
}

internal void fs_dltbitmap(bitmap_t bitmap) // destroys bitmap
{
    if (!bitmap)
        return;

    free(bitmap);
    return;
}

// returns 0 either when filesys is NULL or when no free block found on the drive
internal uint16_t fs_first_free(filesys_t *filesys)
{
    bitmap_t bitmap;
    uint16_t index, size;
    drive_t *drive;

    if (!filesys)
        return 0;

    bitmap = filesys->bitmap;
    if (!bitmap)
        return 0;

    drive = filesys->drive;
    size = drive->blocks; // total number of blocks in the drive (filesystem)

    index = 0;
    while (index < size && get_bit(bitmap, index))
        index++;

    return (index >= size) ? 0 : index; // return 0 when no free block found
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
        if (!d_write(drive, buf, i)) // zeroing a inode makes it of type TYPE_NOT_VALID, which is what we want
        {
            free(filesys->bitmap);
            free(filesys);
            return NULL;
        }
    }

    // create initial bitmap
    filesys->bitmap = fs_mkbitmap(filesys, true);
    if (!filesys->bitmap)
    {
        free(filesys);
        return NULL;
    }

    return filesys;
}