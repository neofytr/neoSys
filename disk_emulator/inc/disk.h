#include <base.h>
#include <stdint.h>

internal struct packed neo_disk
{
    int fd;            // the file descriptor backing this disk
    uint16_t blocks;   // number of blocks in the disk
    bool attached : 1; // just one bit; indicates if the disk is attached or not
    uint8_t drive;     // drive number
};

neo_disk *d_attach(uint8_t drive_num);