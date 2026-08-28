# User-mode API

Header: `kernel/include/kernel/user_mode.h`

`user_mode_prepare()` allocates, zeroes, and maps the fixed initial user code
and stack pages. Their addresses are `USER_CODE_VIRTUAL` (`0x00800000`) and
`USER_STACK_VIRTUAL` (`0x00802000`); the page between them is a guard page.
`USER_STACK_TOP` is the initial user stack pointer.

`user_mode_enter(entry, stack_top)` is non-returning. It loads user data
selectors, builds a privilege-changing `iret` frame, enables IF in the
returned flags, and enters ring 3. The current prepared code stores
`Hello from ring 3\n` at offset `0x100` in its code page, then demonstrates a
pointer-based syscall: it supplies the buffer address in `EBX` and byte count
in `ECX`, calls `SYSCALL_WRITE`, copies EAX's return value back into `EBX`, and
calls `SYSCALL_TEST` to log the byte count. It then calls
`SYSCALL_GET_TICKS`, passes its EAX return value to `SYSCALL_TEST`, and loops.
The same program can be pointed at the guard page to demonstrate the
invalid-buffer error path. This is a controlled ABI demonstration rather than
a general process lifecycle.
