# User-copy API

Header: `kernel/include/kernel/usercopy.h`

`copy_from_user(kernel_destination, user_source, length)` copies exactly
`length` bytes from a ring-3 virtual address into trusted kernel memory. It
first verifies the complete source range with
`paging_is_user_range_accessible(..., false)`, so the source must be present
and user-accessible across every touched page.

It returns `false` without copying when the user range is invalid. A
zero-length copy succeeds without accessing either source memory or the
destination.

The caller owns the destination: it must provide valid kernel-writable storage
large enough for `length` bytes. The helper currently supports copying *from*
user memory only; a future `copy_to_user()` will require user-writable range
validation.

`SYSCALL_WRITE` uses this API to copy a user buffer into a kernel-local buffer
before handing it to the serial driver. This keeps drivers from receiving raw,
untrusted user pointers.
