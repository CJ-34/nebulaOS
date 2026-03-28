#ifndef _KERNEL_ASSERT_H
#define _KERNEL_ASSERT_H 

#include <kernel/panic.h>

#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            panicf_impl( \
                __FILE__, \
                __LINE__, \
                __func__, \
                "Assertion failed: %s", \
                #expr \
            ); \
        } \
    } while(0)

#define UNREACHABLE() \
    panic_impl( \
        "Reached unreachable code", \
        __FILE__, \
        __LINE__, \
        __func__ \
    ) 

#endif