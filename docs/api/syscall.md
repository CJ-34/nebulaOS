# Syscall API

Header: `kernel/include/kernel/syscall.h`

`syscall_handler()` is the current C target of the ring-3 `int $0x80` entry
stub. It only logs that user mode reached the kernel, then returns through the
assembly stub's `iret`.

There are currently no syscall numbers, arguments, return values, validation,
or process-exit semantics. A future syscall frame will expose saved registers
to the handler.
