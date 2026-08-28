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
- `stack_pointer`: the saved ESP used by the cooperative context switch.
- `entry`: the C function entered the first time a newly created task runs.

`TASK_MAX_COUNT` is currently 16. `task_system_init(page_directory, stack_top)`
clears the fixed static task table and creates task ID 0 in `TASK_RUNNING`
state. `task_current()` returns that task.

`task_create(page_directory)` uses the first `TASK_UNUSED` slot, assigns the
next ID, reserves that slot's 16 KiB statically allocated kernel stack, and
returns the task in `TASK_READY` state. Its stack top is 16-byte aligned. The
boot task continues to use the bootstrap stack from `boot.S`. `task_create()`
returns `NULL` when the table is full.

The static stack array deliberately trades memory for simplicity: up to 16
task slots reserve their kernel stacks in `.bss`. These stacks are used by the
current cooperative kernel-task switch; later work will make them available to
preemptive and user-mode tasks.

## Cooperative switching

`task_yield()` finds a `TASK_READY` task, marks the current task ready, marks
the selected task running, and calls the i386 `task_switch` assembly routine.
The routine saves EBP, EBX, ESI, EDI, and ESP on the old task's stack; it then
restores those values from the new task's saved stack and uses `ret` to resume
execution. EIP is therefore represented by the return address on each saved
stack, not a separate task field.

A new task starts with a synthetic saved-register frame whose return address
enters an internal bootstrap function. The bootstrap invokes `task->entry`.
Task entry functions must not return; the current bootstrap halts forever if
one does.

All tasks currently share one page directory. There is no CR3 switch, exit
path, runnable queue, round-robin policy, interrupt masking, or PIT-driven
preemption. `task_yield()` is only safe for the controlled cooperative test
that runs before interrupts are enabled.

The boot path tests this registry by creating one ready worker task, switching
to it, and yielding back to task 0. It confirms that the worker ran, its stack
differs from task 0's stack, and task 0 again becomes current.
