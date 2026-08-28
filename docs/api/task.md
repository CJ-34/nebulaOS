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
- `kernel_stack_top`: the top of the task's kernel stack. When a user task
  enters ring 0, this will eventually supply the TSS `esp0` value.

`TASK_MAX_COUNT` is currently 16. `task_system_init(page_directory, stack_top)`
clears the fixed static task table and creates task ID 0 in `TASK_RUNNING`
state. `task_current()` returns that task.

`task_create(page_directory, stack_top)` uses the first `TASK_UNUSED` slot,
assigns the next ID, and returns it in `TASK_READY` state. It returns `NULL`
when the table is full. Creating a task does not run it or allocate a stack;
the supplied values are metadata only until context switching is implemented.

The boot path tests this registry by creating one ready task while confirming
that task 0 remains current. The test tasks share the bootstrap stack only
because the ready task is never scheduled. Saved CPU registers intentionally
do not exist in this structure yet; they belong to the later context-switch
milestone.
