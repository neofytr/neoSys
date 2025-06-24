#define INSIDE_SYS

#include <sys.h>
#include <stdbool.h>
#include <errnum.h>

#define MAX_FD (1U << 8)

// variables private to the syscall api
private int file_desc_arr[MAX_FD] = {
    0,
    1,
    2,
    -1,
}; // file_desc_arr[file] is the posix fd mapped to the neoSys fd file
   // it's -1 if  there is no mapping

// the neoSys fd 0, 1, and 2 are the same as the posix fd 0, 1, and 2 respectively
// neoSys fd 0 -> stdin
// neoSys fd 1 -> stdout
// neoSys fd 2 -> stderr

// functions private to the syscall api
private bool isopen(const fd_t file); // check if the file desc file points to an open file

// macro functions private to the syscall api
#define get_posix_fd(x) (file_desc_arr[(x)])

private bool isopen(const fd_t file)
{
    // checks if the neoSys fd file is associated with some open file (this meaning will be clear in production)
#include <sys/stat.h> // for using fstat during building
#include <errno.h>    // for checking the error returned by fstat

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

public bool store(const fd_t file, const uint8_t chr)
{
    int ret;
    int posix_fd;

    if (!isopen(file))
    {
        ret_err(ErrBadFD);
    }

    // get_posix_id won't fail since we have successfuly come out of the isopen function
    posix_fd = get_posix_fd(file);

#include <unistd.h> // for posix read/write function
    ret = write(posix_fd, (void *)&chr, 1);
    if (ret != 1)
    {
        ret_err(ErrIO);
    }

    ret_success;
}

private uint8_t load(const fd_t file)
{
    int ret;
    int posix_fd;
    uint8_t buf;

    if (!isopen(file))
    {
        ret_err(ErrBadFD);
    }

    posix_fd = get_posix_fd(file);

    ret = read(posix_fd, &buf, 1);
    if (ret != 1)
    {
        ret_err(ErrIO);
    }

    ret_success; // exiting from syscall api successfully sets errnum to SUCCESS
}