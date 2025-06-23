#include <sys.h>

#define MAX_OPEN_FILES (256)

// variables private to the syscall api
private bool open_fd[MAX_OPEN_FILES];

// functions private to the syscall api
private bool isopen(fd_t file); // check if the file desc file points to an open file

private bool isopen(fd_t file)
{
}

public uint8_t store(fd_t file, uint8_t chr)
{
    if (file > 1)
    {
        // file descriptors 0 and 1 are always open (stdin and stdout)
        if (!isopen(file))
        {
            reterr(ErrBadFD);
        }
    }
}