#include <disk.h>

// a bit-wise flag to see if a disk is attached or not
// if drive C is attached, it will have it's LSB set
// if drive D is attached, it will have it's 2nd LSB set and so on
internal uint8_t attached = 0;

void d_detach(disk_t *disk)
{
    uint8_t tmp;
    if (!disk)
    {
        return;
    }

    attached &= ~(disk->drive); // turn off the drive number in the attached
    free(disk);
}