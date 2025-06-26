#include <disk.h>
#include <stdlib.h>

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
    if (!(drive_num == DriveC || drive_num == DriveD))
    {
        return NULL;
    }

    disk_t *disk = malloc(sizeof(disk_t));
    if (!disk)
    {
        return NULL;
    }

    uint8_t *file = strnum(DRIVE_BASE_PATH, drive_num);
}