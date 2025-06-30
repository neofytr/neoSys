#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <disk.h>
#include <filesys.h>

void usage(char *arg);
void usage_format(char *arg);
void cmd_format(char *, char *);
int main(int argc, char **argv);

void usage(char *arg)
{
    fprintf(stderr, "Usage: %s <command> [arguments]\n", arg);
    fprintf(stderr, "Available commands:\n"
                    "1. format\n");

    exit(EXIT_FAILURE);
}

void cmd_format(char *arg1, char *arg2)
{
    char drive = 0;
    char *drive_str = NULL;
    drive_t *drive_desc = NULL;
    filesys_t *filesys = NULL;

    bool bootable = false;
    char force = true;
    int ret = 0;

    if (!arg1)
        usage_format("diskutil");
    if (!arg2)
    {
        bootable = false;
        drive_str = arg1;
    }
    else
    {
        if (!strcmp((const char *)arg1, "-s"))
        {
            bootable = true;
            drive_str = arg2;
        }
        else
            usage_format("diskutil");
    }

    switch (*drive_str) // based on the first character of drive_string
    {
    case 'c':
    case 'C':
        drive = DriveC;
        break;
    case 'd':
    case 'D':
        drive = DriveD;
        break;
    default:
        usage_format("diskutil");
        break;
    }

    if (bootable)
    {
        fprintf(stderr, "Bootable drives currently not supported\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "This will format and ERASE your drive %s\n", drive_str);
    fprintf(stdout, "Continue? (y/n): ");

    ret = scanf("%c", &force);
    if (ret < 1)
    {
        fprintf(stderr, "Error reading the choice\n");
        return;
    }

    force = (force == 'y' || (char)force == 'Y') ? 1 : 0;
    if (!force)
        return;

    fprintf(stdout, "Formatting drive %s\n", drive_str);
    fprintf(stdout, "drive: %d\n", drive);
    drive_desc = d_attach(drive);
    if (!drive_desc)
    {
        fprintf(stderr, "Bad drive %s\n", drive_str);
        return;
    }

    filesys = fs_format(drive_desc, NULL, true);
    if (!filesys)
    {
        fprintf(stderr, "Error formatting the drive %s\n", drive_str);
        return;
    }

    return;
}
void usage_format(char *arg)
{
    fprintf(stderr, "Usage: %s format [-s] <drive>\n", arg);
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "%s format C:\n", arg);

    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    char *arg1 = NULL, *arg2 = NULL, *cmd = NULL;

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
