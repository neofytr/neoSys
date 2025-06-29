#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    return EXIT_SUCCESS;
}
