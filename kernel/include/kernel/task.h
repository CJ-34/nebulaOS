#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#include <stdint.h>

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

#endif