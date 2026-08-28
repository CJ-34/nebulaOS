# Task API

Header: `kernel/include/kernel/task.h`

`struct task` is NebulaOS’s initial model of a schedulable execution context.
It currently contains metadata only; no task allocator, runnable queue,
scheduler, context switch, or per-task address space has been implemented.

Each task has:

- `id`: an identifier for diagnostics and future task lookup.
- `state`: one of `TASK_UNUSED`, `TASK_READY`, `TASK_RUNNING`,
  `TASK_BLOCKED`, or `TASK_TERMINATED`.
- `page_directory_physical`: the page-directory physical address to load into
  CR3 when per-task address spaces and context switching are added. It is
  currently the shared kernel page directory.
- `kernel_stack_top`: the top of the task's kernel stack. When a user task
  enters ring 0, this will eventually supply the TSS `esp0` value.

The boot path constructs a local task with ID 0 and `TASK_RUNNING` to verify
the model after paging is initialized. Saved CPU registers intentionally do
not exist in this structure yet; they belong to the later context-switch
milestone.
