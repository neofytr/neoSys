#include <sys.h>

#define MAX_FD (1U << 8)

// variables private to the syscall api
private int file_desc_arr[MAX_FD]; // file_desc_arr[file] is the posix fd mapped to the neoSys fd file
                                   // it's -1 if there is no mapping

// functions private to the syscall api
private bool isopen(fd_t file); // check if the file desc file points to an open file

// macro functions private to the syscall api
#define get_posix_fd(x) (file_desc_arr[(x)])

private bool isopen(fd_t file)
{
    // checks if the neoSys fd file is associated with some open file (this meaning will be clear in production)
#include <sys/stat.h> // for using fstat during building
#include <errno.h>    // for checking the error returned by fstat

    if (file < 2)
    {
        return true; // neoSys fd 0 and 1 are always open as stdin and stdout
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