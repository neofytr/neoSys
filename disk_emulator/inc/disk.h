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
    int fd;          // the file descriptor backing this drive
    uint16_t blocks; // number of blocks in the drive
    uint8_t drive;   // drive number
} drive_t;

public
void test();
internal drive_t *d_attach(uint8_t drive_num);
internal bool d_detach(drive_t *drive);
internal void d_show(drive_t *drive);
internal bool d_read(drive_t *drive, uint8_t *dest, uint16_t block_num);
internal bool d_write(drive_t *drive, uint8_t *src, uint16_t block_num);

#endif