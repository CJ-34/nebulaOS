# Exceptions API

Header: `kernel/include/kernel/exceptions.h`

`struct exception_frame` matches the registers pushed by the current common
exception stub: `pusha` registers, vector, error code, and CPU return state.

`exception_handler(frame)` is non-returning and turns supported CPU exceptions
into a kernel panic. Page faults also report the faulting linear address from
`CR2`.

For a ring-3 exception, the CPU also pushes user `ESP` and `SS`; those fields
are not yet represented by this structure.
