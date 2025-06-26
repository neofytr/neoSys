#include <disk.h>
#include <osapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>    // for open()
#include <unistd.h>   // for close()
#include <sys/stat.h> // for fstat()

// a bit-wise flag to see if a disk is attached or not
// if drive C is attached, it will have it's LSB set
// if drive D is attached, it will have it's 2nd LSB set and so on
internal uint8_t attached = 0;

public void test()
{
    disk_t *disk = d_attach(DriveC);
    d_show(disk);

    return;
}

internal void d_show(disk_t *disk)
{
    if (!disk)
    {
        fprintf(stdout, "Error -> Invalid disk argument\n");
        return;
    }

    fprintf(stdout, "Disk Info:\n");
    fprintf(stdout, "  File Descriptor : %d\n", disk->fd);
    fprintf(stdout, "  Number of Blocks: %u\n", disk->blocks);
    fprintf(stdout, "  Drive Number    : %u\n", disk->drive);

    return;
}

internal bool d_detach(disk_t *disk)
{
    if (!disk)
    {
        return false;
    }

    attached &= ~(disk->drive); // turn off the drive number in the attached
    close(disk->fd);
    free(disk);

    return true;
}

internal disk_t *d_attach(uint8_t drive_num)
{
    disk_t *disk;
    uint8_t *file;
    int ret;
    struct stat sbuf;

    if (!(drive_num == DriveC || drive_num == DriveD))
    {
        return NULL;
    }

    if (attached & drive_num)
    {
        // if already attached, return error
        return NULL;
    }

    disk = malloc(sizeof(disk_t));
    if (!disk)
    {
        return NULL;
    }

    file = strnum((uint8_t *)DRIVE_BASE_PATH, drive_num);
    if (!file)
    {
        free(disk);
        return NULL;
    }

    printf("%s\n", file);

    ret = open((const char *)file, O_RDONLY);
    if (ret < 0)
    {
        free(disk);
        perror("open");
        return NULL;
    }
    disk->fd = ret;

    ret = fstat(disk->fd, &sbuf);
    if (ret < 0)
    {
        close(disk->fd);
        free(disk);
        return NULL;
    }

    disk->blocks = (uint16_t)sbuf.st_blocks; // it's the number of 512B blocks allocated for the file
    disk->drive = drive_num;
    attached |= drive_num;

    return disk;
}