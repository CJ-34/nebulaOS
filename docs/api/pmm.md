# Physical memory manager API

Header: `kernel/include/kernel/pmm.h`

`pmm_init(mbi)` parses the Multiboot memory map and initializes the 4 KiB
frame bitmap. It reserves low memory, the kernel image, Multiboot data, and
other known kernel-owned ranges.

`pmm_allocate_frame()` returns an aligned physical frame or
`PMM_INVALID_FRAME` on exhaustion. `pmm_free_frame(physical_address)` returns
a frame previously allocated by the PMM. `pmm_total_frames()` and
`pmm_free_frames()` report bitmap statistics.

Do not free arbitrary available or reserved physical addresses: the PMM does
not yet track permanent ownership separately from dynamic allocation.
