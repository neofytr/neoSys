#include <osapi.h>
#include <errno.h>    // for checking the error returned by fstat
#include <sys/stat.h> // for using fstat during building

internal bool isopen(const fd_t file)
{
    // checks if the neoSys fd file is associated with some open file (this meaning will be clear in production)

    if (file < 3)
    {
        return true; // neoSys fd 0, 1 and 2 are always open as stdin, stdout, and stderr respectively
    }

    // each neoSys fd is currently backed by a posix fd
    // we first check if the neoSys fd file is backed by some posix fd
    // if not, we return false

    // if it is, we see if that posix fd is backed by some open file, if yes, we return true
    // or otherwise, we return false

    int posix_fd = get_posix_fd(file); // returns the posix fd mapped to the neoSys fd file
                                       // returns -1 if there is no mapping from the neoSys fd file to a posixfd
    if (posix_fd == -1)
    {
        return false;
    }

    struct stat statbuf;
    // now we check if the mapped posix fd has an open file associated with it
    if (fstat(posix_fd, &statbuf) == -1 && errno == EBADF)
    {
        // fstat can possibly fail with EBADF or ENOMEM
        // ENOMEM means the kernel is out of memory (very unlikely)
        return false;
        // EBADF means bad file descriptor
    }

    return true;
}

internal bool zero(void *mem, uint16_t bytes)
{
    if (!mem)
    {
        return false;
    }

    uint8_t *arr = (uint8_t *)mem;
    for (uint16_t index = 0; index < bytes; index++)
    {
        arr[index] = 0;
    }

    return true;
}