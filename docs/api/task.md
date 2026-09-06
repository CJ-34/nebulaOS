# Task API

Header: `kernel/include/kernel/task.h`

`struct task` is NebulaOS’s initial model of a schedulable execution context.
It has a fixed task table and a cooperative context switch. There is still no
runnable queue or per-task address space.

Each task has:

- `id`: an identifier for diagnostics and future task lookup.
- `state`: one of `TASK_UNUSED`, `TASK_READY`, `TASK_RUNNING`,
  `TASK_BLOCKED`, or `TASK_TERMINATED`.
- `page_directory_physical`: the page-directory physical address to load into
  CR3 when per-task address spaces and context switching are added. It is
  currently the shared kernel page directory.
- `kernel_stack_top`: the top of the task's private kernel stack. The
  scheduler copies the incoming task's value to the TSS `esp0` field before
  that task runs.
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

`task_yield()` selects the next `TASK_READY` task by scanning forward from the
current task's table slot and wrapping at the end of the table. This is a
cooperative round-robin policy: it avoids repeatedly preferring low-index
slots. It marks the current task ready, marks the selected task running,
updates TSS `esp0` for the selected task, and calls the i386 `task_switch`
assembly routine.
The routine saves EFLAGS; the data-segment selectors DS, ES, FS, and GS; and
the callee-saved registers EBP, EBX, ESI, and EDI on the old task's stack. It
then restores them from the new task's saved stack and uses `ret` to resume
execution. EIP is therefore represented by the return address on each saved
stack, not a separate task field. Saving EFLAGS matters because an `int`
syscall through an interrupt gate clears IF; restoring the console task's
saved EFLAGS re-enables keyboard and timer interrupts when that task resumes.

A new task starts with a synthetic frame matching this exact restore order.
Its initial EFLAGS are copied from the creating task, and its initial segment
selectors are the kernel data selector. Its return address enters an internal
bootstrap function. The bootstrap invokes `task->entry`; if the entry returns,
the bootstrap calls `task_exit()`.

`task_exit()` is non-returning. It marks the current task
`TASK_TERMINATED`, selects a ready successor, marks that task running, and
context-switches away. A terminated task cannot be selected again because the
scheduler considers only `TASK_READY` tasks. If no ready successor exists,
the CPU disables interrupts and halts forever.

`task_reap(task)` completes the current task lifecycle. It returns `false` for
`NULL`, the current task, or a task that is not `TASK_TERMINATED`. Otherwise it
clears the task metadata, changing the slot to `TASK_UNUSED`, and returns
`true`. The slot's static stack allocation remains reserved but is reused by
the next task created in that slot. Reaped tasks receive a new monotonically
increasing task ID when recreated.

All tasks currently share one page directory. There is no CR3 switch,
runnable queue, stack deallocation, interrupt-safe scheduling, or PIT-driven
preemption. The PIT does not schedule, so the console may cooperatively yield
to the controlled user-task test; task operations are not yet safe for a
preemptive scheduler or arbitrary interrupt-handler use.

The boot path tests this policy with two workers. Worker A yields, Worker B
runs and terminates, task 0 resumes, then task 0 yields again so Worker A
resumes and terminates. The observed sequence is `boot → A → B → boot → A →
boot`. It then reaps both workers, creates Worker C in the first reusable slot,
and confirms that Worker C runs and terminates with a fresh ID.
