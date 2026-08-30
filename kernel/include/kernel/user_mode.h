#ifndef _KERNEL_USER_MODE_H
#define _KERNEL_USER_MODE_H

#include <kernel/pmm.h>

#include <stdbool.h>
#include <stdint.h>

#define USER_CODE_VIRTUAL 0x00800000u
#define USER_STACK_VIRTUAL 0x00802000u
#define USER_STACK_TOP (USER_STACK_VIRTUAL + PMM_PAGE_SIZE)

bool user_mode_prepare(void);

__attribute__((noreturn))
void user_mode_enter(uint32_t entry, uint32_t stack_top);

__attribute__((noreturn))
void user_mode_task_entry(void);

#endif
