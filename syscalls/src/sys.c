#define INSIDE_SYS

#include <sys.h>
#include <osapi.h>
#include <stdbool.h>
#include <errnum.h>

#include <unistd.h> // for posix read/write function

// the neoSys fd 0, 1, and 2 are the same as the posix fd 0, 1, and 2 respectively
// neoSys fd 0 -> stdin
// neoSys fd 1 -> stdout
// neoSys fd 2 -> stderr

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