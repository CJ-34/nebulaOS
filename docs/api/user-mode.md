# User-mode API

Header: `kernel/include/kernel/user_mode.h`

`user_mode_prepare()` allocates, zeroes, and maps the fixed initial user code
and stack pages. Their addresses are `USER_CODE_VIRTUAL` (`0x00800000`) and
`USER_STACK_VIRTUAL` (`0x00802000`); the page between them is a guard page.
`USER_STACK_TOP` is the initial user stack pointer.

`user_mode_enter(entry, stack_top)` is non-returning. It loads user data
selectors, builds a privilege-changing `iret` frame, enables IF in the
returned flags, and enters ring 3. The current prepared code invokes the
syscall vector and loops, so it does not provide a general process lifecycle.
