#ifndef DISK_H
#define DISK_H
#include <base.h>
#include <stdint.h>
#include <stdbool.h>

// we support only two drives for now, C and D
#define DriveC 0x01 // 0001
#define DriveD 0x20 // 0010

#define DRIVE_BASE_PATH "/home/raj/Desktop/neoSys/disk_emulator/drives/drive."
#define BLOCK_SIZE (512)

typedef struct
{
    int fd;          // the file descriptor backing this disk
    uint16_t blocks; // number of blocks in the disk
    uint8_t drive;   // drive number
} disk_t;

internal disk_t *d_attach(uint8_t drive_num);
internal bool d_detach(disk_t *disk);
internal void d_show(disk_t *disk);

#endif