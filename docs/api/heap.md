# Kernel heap API

Header: `kernel/include/kernel/heap.h`

Call `heap_init()` after paging is enabled. `kmalloc(size)` returns an
8-byte-aligned kernel virtual address or `NULL`; a zero-byte request fails.
`kfree(pointer)` releases a pointer returned by `kmalloc`; `NULL` is accepted.

`heap_used_bytes()` reports the monotonic heap high-water mark, while
`heap_mapped_pages()` reports pages mapped for the heap. The allocator reuses,
splits, and coalesces free blocks, but does not validate pointers, detect
double frees, shrink pages, or synchronize concurrent callers.
