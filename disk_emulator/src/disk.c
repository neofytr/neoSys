#include <disk.h>
#include <osapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>    // for open()
#include <unistd.h>   // for close()
#include <sys/stat.h> // for fstat()

#define is_pow_of_two(num) (!((num) & (num - 1)))

// a bit-wise flag to see if a drive is attached or not
// if drive C is attached, it will have it's LSB set
// if drive D is attached, it will have it's 2nd LSB set and so on
private uint8_t attached = 0;

internal char *d_getdrivename(uint8_t drive_num)
{
    switch (drive_num)
    {
    case DriveC:
        return "DriveC";
    case DriveD:
        return "DriveD";
    default:
        return NULL;
    }
}

public drive_t *drive_test(uint8_t drive_num)
{
    if (drive_num != DriveC && drive_num != DriveD)
        return NULL;

    drive_t *drive = d_attach(drive_num);
    if (!drive)
        return NULL;

    return drive;
}

internal bool d_is_drivenum_valid(uint8_t drive_num)
{
    return (drive_num == DriveC || drive_num == DriveD);
}

internal bool d_read(drive_t *drive, uint8_t *dest, uint16_t block_num)
{
    if (!drive || !dest)
    {
        return false;
    }

    if (lseek(drive->fd, block_num * BLOCK_SIZE, SEEK_SET) < 0)
    {
        return false;
    }

    if (read(drive->fd, (void *)dest, BLOCK_SIZE) < BLOCK_SIZE)
    {
        return false;
    }

    return true;
}

internal bool d_write(drive_t *drive, uint8_t *src, uint16_t block_num)
{
    if (!drive || !src)
    {
        return false;
    }

    if (lseek(drive->fd, block_num * BLOCK_SIZE, SEEK_SET) < 0)
    {
        return false;
    }

    if (write(drive->fd, (void *)src, BLOCK_SIZE) < BLOCK_SIZE)
    {
        return false;
    }

    return true;
}

internal void d_show(drive_t *drive)
{
    if (!drive)
    {
        fprintf(stdout, "Error -> Invalid drive argument\n");
        return;
    }

    fprintf(stdout, "drive Info:\n");
    fprintf(stdout, "  File Descriptor : %d\n", drive->fd);
    fprintf(stdout, "  Number of Blocks: %u\n", drive->blocks);
    fprintf(stdout, "  Drive Number    : %u\n", drive->drive_num);

    return;
}

internal bool d_detach(drive_t *drive)
{
    if (!drive)
    {
        return false;
    }

    attached &= ~(drive->drive_num); // turn off the drive number in the attached
    close(drive->fd);
    free(drive);

    return true;
}

internal drive_t *d_attach(uint8_t drive_num)
{
    drive_t *drive;
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

    drive = malloc(sizeof(drive_t));
    if (!drive)
    {
        return NULL;
    }

    file = strnum((uint8_t *)DRIVE_BASE_PATH, drive_num); // returns static memory (no need to free)
    if (!file)
    {
        free(drive);
        return NULL;
    }

    ret = open((const char *)file, O_RDWR);
    if (ret < 0)
    {
        free(drive);
        perror("open");
        return NULL;
    }
    drive->fd = ret;

    ret = fstat(drive->fd, &sbuf);
    if (ret < 0)
    {
        close(drive->fd);
        free(drive);
        return NULL;
    }

    // the number of blocks we allocate from the file into the drive will be file_size / BLOCK_SIZE
    if (is_pow_of_two(sbuf.st_blocks))
    {
        drive->blocks = sbuf.st_blocks;
    }
    else
    {
        drive->blocks = sbuf.st_blocks - 1;
    }

    drive->drive_num = drive_num;
    attached |= drive_num;

    return drive;
}