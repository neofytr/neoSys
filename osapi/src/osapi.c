#include <osapi.h>

bool internal set(void *mem, uint16_t n, uint8_t c)
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

uint16_t internal stringlen(void *str)
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

uint8_t *internal strnum(uint8_t *str, uint8_t num)
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

bool internal copy(void *dst, void *src, uint16_t n)
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

bool internal zero(void *mem, uint16_t bytes)
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

uint16_t internal openfiles(drive_t *drive)
{
    return 0;
}

void internal erase_all_files(drive_t *drive)
{
    return;
}