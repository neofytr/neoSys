#include <osapi.h>
#include <errno.h>    // for checking the error returned by fstat
#include <sys/stat.h> // for using fstat during building

internal bool set(void *mem, uint16_t n, uint8_t c)
{
    if (!mem)
    {
        return false;
    }

    for (uint16_t index = 0; index < n; index++)
    {
        ((uint8_t *)mem)[index] = c;
    }

    return true;
}

internal uint16_t stringlen(void *str)
{
    if (!str)
    {
        return 0;
    }

    uint16_t len = 0;
    while (*((uint8_t *)str++))
    {
        len++;
    }

    return len;
}

internal uint8_t *strnum(uint8_t *str, uint8_t num)
{
    if (!str)
    {
        return NULL;
    }

    static uint8_t buf[UINT8_MAX] = {0};
    uint16_t len = stringlen(str);

    if (len >= 256)
    {
        return NULL;
    }

    // copy input string to buffer
    for (uint16_t index = 0; index < len; index++)
    {
        buf[index] = str[index];
    }

    // convert number to decimal digits and append
    // handle special case of 0
    if (num == 0)
    {
        if (len < 255) // ensure space for digit and null terminator
        {
            buf[len++] = '0';
        }
    }
    else
    {
        // find the highest power of 10 <= num
        uint8_t temp = num;
        uint16_t divisor = 1;
        while (temp >= 10)
        {
            temp /= 10;
            divisor *= 10;
        }

        // extract digits from most significant to least significant
        while (divisor > 0 && len < 255) // ensure space for null terminator
        {
            uint8_t digit = num / divisor;
            buf[len++] = '0' + digit;
            num %= divisor;
            divisor /= 10;
        }
    }

    // null terminate the result
    buf[len] = '\0';

    return buf;
}

internal bool copy(void *dst, void *src, uint16_t n)
{
    if (!dst || !src || !n)
    {
        return false;
    }

    // for safely handling the overlapping case
    // first copy the source data into a separate buffer first
    // then copy the data from this buffer onto the destination
    uint8_t buf[UINT16_MAX];
    for (uint16_t index = 0; index < n; index++)
    {
        buf[index] = ((uint8_t *)src)[index];
    }

    for (uint16_t index = 0; index < n; index++)
    {
        ((uint8_t *)dst)[index] = buf[index];
    }

    return true;
}

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