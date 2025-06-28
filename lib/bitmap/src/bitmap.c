#include <bitmap.h>
#include <neostd.h>

public bitmap_t *bitmap_create(uint16_t bits)
{
    uint16_t size;
    bitmap_t *bitmap;
    uint8_t *ptr;

    bitmap = allocate(sizeof(bitmap_t));
    if (!bitmap)
        ret_err(ErrNoMem);

    size = bits / 8;
    ptr = allocate(size);
    if (!ptr)
    {
        destroy(bitmap);
        return ret_err(ErrNoMem);
    }

    bitmap->ptr = ptr;
    bitmap->total_bits = bits;

    return bitmap;
}

public void bitmap_destroy(bitmap_t bitmap)
{
    if (!bitmap)
        return;

    destroy(bitmap->ptr);
    destroy(bitmap);
    return;
}