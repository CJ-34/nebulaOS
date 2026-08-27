# Panic API

Header: `kernel/include/kernel/panic.h`

`PANIC(message)` and `PANICF(format, ...)` capture the call-site file, line,
and function, display a panic report, and do not return. The lower-level
`panic_impl`, `panicf_impl`, and `vpanicf_impl` functions provide the same
non-returning behavior for code that already has location or `va_list` data.
