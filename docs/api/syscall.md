# Syscall API

Header: `kernel/include/kernel/syscall.h`

`syscall_handler()` is the current C target of the ring-3 `int $0x80` entry
stub. The stub saves the user data-segment selectors, then saves the
general-purpose registers and passes a pointer to a `struct syscall_frame` to
the handler. Before entering C, it loads the kernel data selector into DS, ES,
FS, and GS. After the handler it restores the registers and original user
selectors, then returns through `iret`.

`frame->eax` contains the syscall number. `SYSCALL_TEST` is number `1`; it
logs the first argument. `SYSCALL_ECHO` is number `2`; it returns that first
argument unchanged. `SYSCALL_WRITE` is number `3`; it writes a validated
user-supplied byte buffer to the serial output. `SYSCALL_GET_TICKS` is number
`4`; it returns the current PIT tick count. `SYSCALL_EXIT` is number `5`; it
terminates the current task and does not return to ring 3. Other syscall
numbers are logged as errors.

The current register ABI is:

- `EAX`: syscall number on entry and return value on return.
- `EBX`: first argument.
- `ECX`: second argument.

The selector saves occur before `pusha`, so ESP still points at
`struct syscall_frame` when that pointer is passed to C. Changing `frame->eax`
changes the EAX register restored by `popa`, so it becomes the value received
by ring 3 after `iret`.

For `SYSCALL_WRITE`, `EBX` is the user virtual address and `ECX` is the byte
count. The count is limited to 128 bytes. Before reading the buffer, the
kernel verifies that the entire range is user-accessible; an invalid range or
excessive length returns `SYSCALL_ERROR_INVALID_ARGUMENT` (`0xFFFFFFFF`) in
EAX. A successful call returns the number of bytes written in EAX. The buffer
is copied into kernel-local memory with `copy_from_user()` before the serial
driver uses it.

`SYSCALL_GET_TICKS` has no input arguments. It reads the current kernel PIT
tick count and returns it in EAX. As a 32-bit counter it eventually wraps; it
is a tick count, not a wall-clock time API.

`SYSCALL_EXIT` has no input arguments or return value. It delegates to
`task_exit()`, which marks the task terminated and cooperatively switches to a
ready successor. The user program must treat it as non-returning.

There are not yet conventions for further arguments or process-level resource
cleanup.
