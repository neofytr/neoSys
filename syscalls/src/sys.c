#include <sys.h>

// functions private to the kernel
private bool isopen(fd_t file); // check if the file desc file points to an open file

static bool isopen(fd_t file)
{
}

uint8_t store(fd_t file, uint8_t chr)
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