#ifndef _KERNEL_SYSCALL_H
#define _KERNEL_SYSCALL_H

#include <stdint.h>

#define SYSCALL_TEST 1u
#define SYSCALL_ECHO 2u
#define SYSCALL_WRITE 3u
#define SYSCALL_GET_TICKS 4u

#define SYSCALL_ERROR_INVALID_ARGUMENT 0xFFFFFFFFu

struct syscall_frame {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_before_pusha;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t user_esp;
    uint32_t user_ss;
} __attribute__((packed));

void syscall_handler(struct syscall_frame *frame);

#endif
