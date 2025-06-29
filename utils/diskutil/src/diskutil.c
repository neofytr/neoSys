#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <disk.h>
#include <filesys.h>

void usage(uint8_t *arg);
void usage_format(uint8_t *arg);
void cmd_format(uint8_t *, uint8_t *);
int main(int argc, uint8_t **argv);

void usage(uint8_t *arg)
{
    fprintf(stderr, "Usage: %s <command> [arguments]\n", arg);
    fprintf(stderr, "Available commands:\n"
                    "1. format\n");

    exit(EXIT_FAILURE);
}

void cmd_format(uint8_t *arg1, uint8_t *arg2)
{
    uint8_t drive;
    uint8_t *drive_str;
    drive_t *drive_desc;
    filesys_t *filesys;

    bool bootable;
    bool force;

    if (!arg1)
        usage_format("diskutil");
    if (!arg2)
    {
        bootable = false;
        drive_str = arg1;
    }
    else
    {
        if (!strcmp(arg1, "-s"))
        {
            bootable = true;
            drive_str = arg2;
        }
        else
            usage_format("diskutil");
    }

    switch (*drive_str)
    {
    case ('c' || 'C'):
        drive = 1;
        break;
    case ('D' || 'd'):
        drive = 2;
        break;
    default:
        usage_format("diskutil");
    } // based on the first character of drive_string

    if (bootable)
    {
        fprintf(stderr, "Bootable drives currently not supported\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "This will format and ERASE your drive %s\n", drive_str);
    fprintf(stdout, "Continue? (y/n)");
    fflush(stdout);

    scanf("%d\n", &force);
    force = (force == 'y' || force == 'Y') ? true : false;
    if (!force)
        return;

    fprintf(stdout, "Formatting drive %s\n", drive_str);
    drive_desc = d_attach(drive);
    if (!drive_desc)
        fprintf(stderr, "Bad drive %s\n", drive_str) && return EXIT_FAILURE;

    filesys = fs_format(drive_desc, NULL, true);
    if (!filesys)
        fprintf(stderr, "Error formatting the drive %s\n", drive_str) && return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
void usage_format(uint8_t *arg)
{
    fprintf(stderr, "Usage: %s format [-s] <drive>\n", arg);
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "%s format C:\n", arg);

    exit(EXIT_FAILURE);
}

int main(int argc, uint8_t **argv)
{
    uint8_t *arg1, *arg2, *cmd;

    if (argc < 2)
        usage(argv[0]);

    cmd = argv[1];
    if (argc == 2)
        arg1 = arg2 = NULL;
    else if (argc == 3)
        arg1 = argv[2];
    else
    {
        arg1 = argv[2];
        arg2 = argv[3];
    }

    if (!strcmp(cmd, "format"))
        cmd_format(arg1, arg2);
    else
        usage(argv[0]);

    return EXIT_SUCCESS;
}
