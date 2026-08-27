# Logging API

Header: `kernel/include/kernel/log.h`

Call `log_init()` before emitting logs. `log_write(level, format, ...)` writes
formatted output to the configured serial logger. Convenience macros include
`log_debug`, `log_info`, `log_error`, and the currently named `log_warm`
macro (the latter is a spelling mistake retained by the current API).

Logging is diagnostic infrastructure; it is not synchronized for concurrent
use from multiple execution contexts.
