#include <disk.h>
#include <stdlib.h>
#include <fcntl.h>    // for open()
#include <sys/stat.h> // for fstat()

// a bit-wise flag to see if a disk is attached or not
// if drive C is attached, it will have it's LSB set
// if drive D is attached, it will have it's 2nd LSB set and so on
internal uint8_t attached = 0;

internal bool d_detach(disk_t *disk)
{
    uint8_t tmp;
    if (!disk)
    {
        return;
    }

    attached &= ~(disk->drive); // turn off the drive number in the attached
    close(disk->fd);
    free(disk);

    return;
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

    file = strnum(DRIVE_BASE_PATH, drive_num);
    if (!file)
    {
        free(disk);
        return NULL;
    }

    ret = open(file, O_RDONLY);
    if (ret < 0)
    {
        free(disk);
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

    disk->blocks = sbuf.st_blocks; // it's the number of 512B blocks allocated for the file
    disk->drive = drive_num;
    attached |= drive_num;

    return disk;
}