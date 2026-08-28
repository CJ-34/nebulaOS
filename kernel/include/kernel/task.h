#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#include <stdint.h>

#define TASK_MAX_COUNT 16u

enum task_state {
    TASK_UNUSED,
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_TERMINATED,
};

struct task {
    uint32_t id;
    enum task_state state;

    uint32_t page_directory_physical;
    uint32_t kernel_stack_top;
};

void task_system_init(
    uint32_t boot_page_directory,
    uint32_t boot_kernel_stack_top
);

struct task *task_current(void);

struct task *task_create(
    uint32_t page_directory,
    uint32_t kernel_stack_top
);

#endif