#include <filesys.h>
#include <osapi.h>
#include <errnum.h>
#include <stdlib.h>
#include <stdio.h>

#define BUF_LEN_FOR_FILENAME (13) // 8 (name) + 3 (extension) + 1 (dot) + 1 (null byte)

private uint16_t find_free_block(bitmap_t bitmap, uint16_t total_blocks);
private bool mark_block_used(bitmap_t bitmap, uint16_t block_num);
private void mark_block_free(bitmap_t bitmap, uint16_t block_num);
private bool get_file_name(inode_t *inode, uint8_t *name);

public void
filesys_test(drive_t *drive)
{
    filesys_t *filesys = NULL;
    if (!(filesys = fs_format(drive, NULL, true)))
        return;

    fs_show(filesys, true);
}

internal bool fs_get_inode(filesys_t *filesys, uint16_t inode_index, inode_t *inode)
{
    uint16_t inode_blocks;
    uint16_t inode_index_in_block;
    uint16_t inode_block_index;
    datablock_t inode_block;

    if (!filesys || !inode)
        return false;

    inode_blocks = filesys->super_block.inode_blocks;
    inode_block_index = inode_index / INODES_PER_BLOCK;
    if (inode_block_index >= inode_blocks)
        return false;
    inode_block_index++; // inode blocks start at block index 1, after the superblock (block index 0)
    inode_index_in_block = inode_index % INODES_PER_BLOCK;

    if (!d_read(filesys->drive, (uint8_t *)&inode_block.data, inode_block_index))
        return false;

    copy((void *)inode, (void *)&inode_block.inode[inode_index_in_block], sizeof(inode_t));
    return true;
}

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

// buf should be of atleast 13 bytes
private bool get_file_name(inode_t *inode, uint8_t *buf)
{
    uint8_t index;
    uint8_t *ext;
    uint8_t *name;
    if (!inode || !buf)
        return false;

    index = 0;
    ext = inode->file_name.extension;
    name = inode->file_name.name;

    while (index < FILEEXT_LEN && *ext)
        buf[index++] = *ext++;
    buf[index++] = '.';
    while (index < BUF_LEN_FOR_FILENAME && *name)
        buf[index++] = *name++;
    buf[index] = '\0';
    return true;
}

internal void fs_show(filesys_t *filesys, bool show_bitmap)
{
    uint16_t i, j, used_blocks, free_blocks;
    uint16_t total_used_inodes, index;
    bitmap_t bitmap;
    inode_t inode;
    uint8_t buf[BUF_LEN_FOR_FILENAME];

    if (!filesys)
        return;

    // print filesystem header
    printf("filesystem information:\n");
    printf("======================\n");

    // print superblock info
    printf("drive number: %d\n", filesys->drive_num);
    printf("total blocks: %d\n", filesys->super_block.blocks);
    printf("inode blocks: %d\n", filesys->super_block.inode_blocks);
    printf("total inodes: %d\n", filesys->super_block.inodes);
    printf("magic numbers: 0x%04x 0x%04x\n", filesys->super_block.magic1, filesys->super_block.magic2);

    // print all inodes
    printf("\n");
    printf("inode table:\n");
    printf("============\n");

    total_used_inodes = filesys->super_block.inodes;
    for (index = 0; index < total_used_inodes; index++)
    {
        if (!fs_get_inode(filesys, index, &inode))
            return;

        if (!get_file_name(&inode, buf))
            return;
        printf("inode_index %d: type=%d, file_size=%d (bytes), file_name=%s\n", index, inode.file_type, inode.file_size, (char *)buf);
    }

    printf("\n");

    // calculate used/free blocks from bitmap
    bitmap = filesys->bitmap;
    used_blocks = 0;
    free_blocks = 0;

    if (bitmap)
    {
        for (i = 0; i < filesys->super_block.blocks; i++)
        {
            if (get_bit(bitmap, i))
                used_blocks++;
            else
                free_blocks++;
        }
    }

    printf("used blocks: %d\n", used_blocks);
    printf("free blocks: %d\n", free_blocks);

    // show bitmap if requested
    if (show_bitmap && bitmap)
    {
        printf("\nblock bitmap (0=free, 1=used):\n");
        printf("==============================\n");

        for (i = 0; i < filesys->super_block.blocks; i += 16)
        {
            printf("%04d: ", i);
            for (j = 0; j < 16 && (i + j) < filesys->super_block.blocks; j++)
            {
                printf("%d", get_bit(bitmap, i + j) ? 1 : 0);
            }
            printf("\n");
        }
    }

    printf("\n");
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