#include <filesys.h>
#include <osapi.h>
#include <errnum.h>
#include <stdlib.h>

#include <filesys.h>
#include <osapi.h>
#include <errnum.h>
#include <stdlib.h>

internal filesys_t *fs_format(drive_t *drive, bootsec_t *boot_sector, bool force)
{
    filesys_t *filesys;
    inode_t root_inode;
    uint16_t inode_blocks;
    uint16_t blocks;
    uint8_t buf[BLOCK_SIZE];

    // basic validation
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
    if (!(filesys = (filesys_t *)malloc(sizeof(filesys))))
        return NULL;

    // calculate inode blocks (10% of total, rounded up)
    inode_blocks = filesys->super_block.inode_blocks = (drive->blocks + 9) / 10;

    // initialize superblock directly
    filesys->super_block.magic1 = MAGIC1;
    filesys->super_block.magic2 = MAGIC2;
    filesys->super_block.inodes = 1;
    blocks = filesys->super_block.blocks = drive->blocks;
    filesys->super_block.reserved = 0;

    // copy or zero boot sector
    if (boot_sector)
    {
        copy((void *)&filesys->super_block.boot_sector, (void *)boot_sector, BOOT_SECTOR_SIZE);
    }
    else
    {
        zero((void *)&filesys->super_block.boot_sector, BOOT_SECTOR_SIZE);
    }

    // set drive info
    filesys->drive = drive;
    filesys->drive_num = drive->drive_num;

    zero((void *)&filesys->bitmap, sizeof(filesys->bitmap));

    // write superblock to drive (assuming block 0)
    if (!d_write(drive, &filesys->super_block, 0))
    {
        free(filesys);
        return NULL;
    }

    root_inode.file_size = 0;
    root_inode.file_type = TYPE_DIR;
    root_inode.indirect_ptr = NULL;
    zero((void *)&root_inode.direct_ptr, sizeof(root_inode.direct_ptr));
    zero((void *)&root_inode.file_name, sizeof(root_inode.file_name));

    // write root inode to the first inode block (2nd overall block)
    zero((void *)buf, BLOCK_SIZE);
    copy((void *)buf, (void *)root_inode, sizeof(root_inode));
    if (!d_write(drive, buf, 1))
    {
        free(filesys);
        return NULL;
    }

    // zero all the other inode blocks
    zero((void *)buf, BLOCK_SIZE);
    for (uint16_t index = 1; block < inode_blocks; index++)
    {
        if (!d_write(drive, buf, 1 + index))
        {
            free(filesys);
            return NULL;
        }
    }

    // zero all the other remaining blocks
    for (uint16_t index = inode_blocks + 1, index < blocks; index++)
    {
        if (!d_write(drive, buf, index))
        {
            free(filesys) return NULL;
        }
    }

    return filesys;
}