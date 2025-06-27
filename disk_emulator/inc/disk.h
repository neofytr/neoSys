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
    int fd;            // the file descriptor backing this drive
    uint16_t blocks;   // number of blocks in the drive; blocks are numbered from 0 to blocks - 1
    uint8_t drive_num; // drive number
} drive_t;

public
void test();
internal drive_t *d_attach(uint8_t drive_num);
internal bool d_detach(drive_t *drive);
internal void d_show(drive_t *drive);
internal bool d_read(drive_t *drive, uint8_t *dest, uint16_t block_num);
internal bool d_write(drive_t *drive, uint8_t *src, uint16_t block_num);

// this will be true if and only if all the three statements return true
// which is possible only if all the stuff happens correctly
// so, this behaves the same way as d_read and d_write
#define dio(func, drive, ptr, num) (drive) &&                                                    \
                                       (lseek((drive)->fd, BLOCK_SIZE * num, SEEK_SET) != -1) && \
                                       (func((drive)->fd, ptr, BLOCK_SIZE) == BLOCK_SIZE)
#define dread(drive, dest, block_num) dio(read, drive, dest, block_num)
#define dwrite(drive, src, block_num) dio(write, drive, src, block_num)

#endif