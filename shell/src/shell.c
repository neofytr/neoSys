#include <shell.h>
#include <syscalls.h>
#include <disk.h>
#include <errnum.h>
#include <filesys.h>
#include <disk.h>

int main()
{
    drive_t *drive = drive_test(DriveC);
    if (!drive)
        return 1;

    filesys_test(drive);
    return 0;
}