#ifndef _KERNEL_PANIC_H
#define _KERNEL_PANIC_H

#include <stdarg.h>

__attribute__((noreturn))
void panic_impl(
    const char* message,
    const char* file,
    int line,
    const char* func
);

__attribute__((noreturn)) 
void panicf_impl(
    const char* file,
    int line,
    const char* func,
    const char* fmt,
    ...
);

__attribute__((noreturn))
void vpanicf_impl(
    const char* file,
    int line,
    const char* func,
    const char* fmt,
    va_list args
);

#define PANIC(msg) \
    panic_impl((msg), __FILE__, __LINE__, __func__)

#define PANICF(fmt, ...) \
    panicf_impl(__FILE__, __LINE__, __func__, (fmt), ##__VA_ARGS__)

#endif