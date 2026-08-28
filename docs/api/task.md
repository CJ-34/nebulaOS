# Task API

Header: `kernel/include/kernel/task.h`

`struct task` is NebulaOS’s initial model of a schedulable execution context.
It currently contains metadata only; no runnable queue, scheduler, context
switch, or per-task address space has been implemented.

Each task has:

- `id`: an identifier for diagnostics and future task lookup.
- `state`: one of `TASK_UNUSED`, `TASK_READY`, `TASK_RUNNING`,
  `TASK_BLOCKED`, or `TASK_TERMINATED`.
- `page_directory_physical`: the page-directory physical address to load into
  CR3 when per-task address spaces and context switching are added. It is
  currently the shared kernel page directory.
- `kernel_stack_top`: the top of the task's private kernel stack. When a user
  task enters ring 0, this will eventually supply the TSS `esp0` value.

`TASK_MAX_COUNT` is currently 16. `task_system_init(page_directory, stack_top)`
clears the fixed static task table and creates task ID 0 in `TASK_RUNNING`
state. `task_current()` returns that task.

`task_create(page_directory)` uses the first `TASK_UNUSED` slot, assigns the
next ID, reserves that slot's 16 KiB statically allocated kernel stack, and
returns the task in `TASK_READY` state. Its stack top is 16-byte aligned. The
boot task continues to use the bootstrap stack from `boot.S`. `task_create()`
returns `NULL` when the table is full.

The static stack array deliberately trades memory for simplicity: up to 16
task slots reserve their kernel stacks in `.bss`. These stacks are currently
never switched to; they become active only during the later context-switch
milestone.

The boot path tests this registry by creating one ready task, confirming that
its stack differs from task 0's stack, and confirming that task 0 remains
current. Saved CPU registers intentionally do not exist in this structure yet;
they belong to the later context-switch milestone.
