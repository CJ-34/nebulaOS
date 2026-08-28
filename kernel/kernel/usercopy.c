#include <string.h>

#include <kernel/paging.h>
#include <kernel/usercopy.h>

bool copy_from_user(
    void *kernel_destination,
    uint32_t user_source,
    uint32_t length
) {
    if (length == 0)
    {
        return true;
    }

    if (!paging_is_user_range_accessible(
        user_source,
        length,
        false))
    {
            return false;
    }

    memcpy(kernel_destination, (const void *)user_source, length);

    return true;
}