#ifndef _KERNEL_USERCOPY_H
#define _KERNEL_USERCOPY_H

#include <stdbool.h>
#include <stdint.h>

bool copy_from_user(
    void *kernel_destination,
    uint32_t user_source,
    uint32_t length
);

#endif
