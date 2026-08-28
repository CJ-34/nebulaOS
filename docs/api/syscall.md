# Syscall API

Header: `kernel/include/kernel/syscall.h`

`syscall_handler()` is the current C target of the ring-3 `int $0x80` entry
stub. The stub saves the general-purpose registers, passes a pointer to a
`struct syscall_frame` to the handler, and returns through `iret`.

`frame->eax` contains the syscall number. `SYSCALL_TEST` is number `1`; it
logs the first argument. `SYSCALL_ECHO` is number `2`; it returns that first
argument unchanged. Other syscall numbers are logged as errors.

The current register ABI is:

- `EAX`: syscall number on entry and return value on return.
- `EBX`: first argument.

The assembly entry stub saves registers before calling C. Changing
`frame->eax` changes the EAX register restored by `popa`, so it becomes the
value received by ring 3 after `iret`.

There are not yet conventions for further arguments, pointer validation, or
process-exit semantics.
