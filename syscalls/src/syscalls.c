#define INSIDE_SYS

#include <syscalls.h>
#include <osapi.h>
#include <stdbool.h>
#include <errnum.h>

#include <errno.h>    // for checking the error returned by fstat
#include <sys/stat.h> // for using fstat during building
#include <unistd.h>   // for posix read/write functions

// macro to get the posix fd corresponding to a neosys fd
#define get_posix_fd(x) (file_desc_arr[(x)])

// the neoSys fd 0, 1, and 2 are the same as the posix fd 0, 1, and 2 respectively
// neoSys fd 0 -> stdin
// neoSys fd 1 -> stdout
// neoSys fd 2 -> stderr

public bool isopen(const fd_t file)
{
    // checks if the neoSys fd file is associated with some open file (this meaning will be clear in production)

    if (file < 3)
    {
        ret_success; // neoSys fd 0, 1 and 2 are always open as stdin, stdout, and stderr respectively
    } // checks if the given neosys file descriptor points to an open file

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
        ret_err(ErrBadFD);
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

    ret = write(posix_fd, (void *)&chr, 1);
    if (ret != 1)
    {
        ret_err(ErrIO);
    }

    ret_success;
}

public uint8_t load(const fd_t file)
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