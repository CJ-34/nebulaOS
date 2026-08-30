#include <stddef.h>
#include <string.h>

#include <kernel/task.h>

extern void task_switch(
    uint32_t *old_stack_pointer,
    uint32_t new_stack_pointer
);

static uint8_t task_kernel_stacks[TASK_MAX_COUNT][TASK_KERNEL_STACK_SIZE] __attribute__((aligned(16)));
static struct task tasks[TASK_MAX_COUNT];
static struct task *current_task;
static uint32_t next_task_id;

static __attribute__((noreturn)) void task_bootstrap(void)
{
    current_task->entry();
    task_exit();
}

static uint32_t task_prepare_initial_stack(uint32_t stack_top) {
    uint32_t *stack = (uint32_t *)stack_top;

    *--stack = 0;
    *--stack = (uint32_t)task_bootstrap;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;
    *--stack = 0;

    return (uint32_t)stack;
}

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

struct task *task_create(uint32_t page_directory, task_entry_t entry)
{
    if (entry == NULL) {
        return NULL;
    }

    for (uint32_t i = 1; i < TASK_MAX_COUNT; i++)
    {
        if (tasks[i].state != TASK_UNUSED)
        {
            continue;
        }

        tasks[i].id = next_task_id++;
        tasks[i].state = TASK_READY;
        tasks[i].page_directory_physical = page_directory;
        tasks[i].kernel_stack_top = (uint32_t)&task_kernel_stacks[i][TASK_KERNEL_STACK_SIZE];
        tasks[i].entry = entry;
        tasks[i].stack_pointer = task_prepare_initial_stack(tasks[i].kernel_stack_top);

        return &tasks[i];
    }

    return NULL;
}

static struct task *task_find_ready(void)
{
    uint32_t current_index =
        (uint32_t)(current_task - tasks);

    for (uint32_t offset = 1; offset < TASK_MAX_COUNT; offset++)
    {
        uint32_t index = (current_index + offset) % TASK_MAX_COUNT;

        if (tasks[index].state == TASK_READY)
        {
            return &tasks[index];
        }
    }

    return NULL;
}

void task_yield(void)
{
    struct task *previous = current_task;
    struct task *next = task_find_ready();

    if (next == NULL)
    {
        return;
    }

    previous->state = TASK_READY;
    next->state = TASK_RUNNING;
    current_task = next;

    task_switch(
        &previous->stack_pointer,
        next->stack_pointer
    );
}

  void task_exit(void)
  {
      struct task *previous = current_task;
      struct task *next;

      previous->state = TASK_TERMINATED;

      next = task_find_ready();

      if (next == NULL)
      {
          for (;;)
          {
              __asm__ volatile("cli; hlt");
          }
      }

      next->state = TASK_RUNNING;
      current_task = next;

      task_switch(
          &previous->stack_pointer,
          next->stack_pointer);

      for (;;)
      {
          __asm__ volatile("cli; hlt");
      }
  }

  bool task_reap(struct task *task)
  {
    if (task == NULL) {
        return false;
    }

    if (task == task_current()) {
        return false;
    }

    if (task->state != TASK_TERMINATED) {
        return false;
    }

    memset(task, 0, sizeof(*task));
    return true;
  }
