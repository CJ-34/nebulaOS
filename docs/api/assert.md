# Assertions API

Header: `kernel/include/kernel/assert.h`

`ASSERT(expression)` evaluates an invariant. If it is false, it invokes the
kernel panic path with the source file, line, function, and expression text.
Use it for kernel bugs and boot-time self-tests, not recoverable input errors.

`UNREACHABLE()` panics when control reaches a path that the kernel design says
must be impossible.
