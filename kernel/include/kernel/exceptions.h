#ifndef _KERNEL_EXCEPTIONS_H
#define _KERNEL_EXCEPTIONS_H

#include <stdint.h>

struct exception_frame {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_before_pusha;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t vector;
    uint32_t error_code;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};

__attribute__((noreturn))
void exception_handler(struct exception_frame* frame);

#endif