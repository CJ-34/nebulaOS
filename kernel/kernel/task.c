#include <stddef.h>
#include <string.h>

#include <kernel/task.h>

static struct task tasks[TASK_MAX_COUNT];
static struct task *current_task;
static uint32_t next_task_id;

void task_system_init(uint32_t boot_page_directory, uint32_t boot_kernel_stack_top)
{
    memset(tasks, 0, sizeof(tasks));

    tasks[0].id = 0;
    tasks[0].state = TASK_RUNNING;
    tasks[0].page_directory_physical = boot_page_directory;
    tasks[0].kernel_stack_top = boot_kernel_stack_top;

    current_task = &tasks[0];
    next_task_id = 1;
}

struct task *task_current(void)
{
    return current_task;
}

struct task *task_create(uint32_t page_directory, uint32_t kernel_stack_top)
{
    for (uint32_t i = 1; i < TASK_MAX_COUNT; i++)
    {
        if (tasks[i].state != TASK_UNUSED)
        {
            continue;
        }

        tasks[i].id = next_task_id++;
        tasks[i].state = TASK_READY;
        tasks[i].page_directory_physical = page_directory;
        tasks[i].kernel_stack_top = kernel_stack_top;

        return &tasks[i];
    }

    return NULL;
}
