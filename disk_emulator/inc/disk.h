#include <base.h>
#include <stdint.h>

// we support only two drives for now, C and D
#define DriveC 0x01 // 0001
#define DriveD 0x20 // 0010

#define DRIVE_BASE_PATH "/home/raj/Desktop/neoSys/disk_emulator/drives/disk."

typedef internal struct packed
{
    int fd;            // the file descriptor backing this disk
    uint16_t blocks;   // number of blocks in the disk
    uint8_t drive : 2; // drive number; only 2 bits (for now, since we only have two drives)
} disk_t;

internal disk_t *d_attach(uint8_t drive_num);
internal void d_detach(disk_t *disk);