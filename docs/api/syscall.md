# Syscall API

Header: `kernel/include/kernel/syscall.h`

`syscall_handler()` is the current C target of the ring-3 `int $0x80` entry
stub. The stub saves the general-purpose registers, passes a pointer to a
`struct syscall_frame` to the handler, and returns through `iret`.

`frame->eax` contains the syscall number. `SYSCALL_TEST` is number `1`; it
logs the first argument. `SYSCALL_ECHO` is number `2`; it returns that first
argument unchanged. `SYSCALL_WRITE` is number `3`; it writes a validated
user-supplied byte buffer to the serial output. Other syscall numbers are
logged as errors.

The current register ABI is:

- `EAX`: syscall number on entry and return value on return.
- `EBX`: first argument.
- `ECX`: second argument.

The assembly entry stub saves registers before calling C. Changing
`frame->eax` changes the EAX register restored by `popa`, so it becomes the
value received by ring 3 after `iret`.

For `SYSCALL_WRITE`, `EBX` is the user virtual address and `ECX` is the byte
count. The count is limited to 128 bytes. Before reading the buffer, the
kernel verifies that the entire range is user-accessible; an invalid range or
excessive length returns `SYSCALL_ERROR_INVALID_ARGUMENT` (`0xFFFFFFFF`) in
EAX. A successful call returns the number of bytes written in EAX.

There are not yet conventions for further arguments or process-exit semantics.
