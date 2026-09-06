# Paging API

Header: `kernel/include/kernel/paging.h`

`paging_prepare_identity_map()` creates the initial 4 MiB identity map.
`paging_enable()` loads `CR3` and sets `CR0.PG`; `paging_is_enabled()` reads
that state back.

`paging_create_address_space()` allocates a new page-directory frame, clears
it, and copies the first two PDEs from the original kernel directory. These
PDEs cover the current kernel identity map and kernel heap. The copied PDEs
share the kernel page tables; they do not copy physical memory. User PDEs are
left absent, so a future task can receive private user mappings.

`paging_activate_address_space(directory)` loads an aligned, non-invalid
directory physical address into CR3. Loading CR3 changes the active page
translation context and flushes normal TLB entries. `paging_active_directory_physical()` reads CR3 and returns its page-frame address.

The caller owns a directory returned by `paging_create_address_space()`. It
may free that frame with `pmm_free_frame()` only after activating another
directory; freeing the directory currently loaded in CR3 would leave the CPU
using reclaimed paging memory.

`paging_map_page(virtual, physical, flags)` maps aligned 4 KiB addresses and
returns false for an existing mapping or allocation failure.
`paging_unmap_page(virtual)` removes an existing mapping. Use
`PAGING_PAGE_WRITABLE` and `PAGING_PAGE_USER` as flags. User mappings set the
U/S bit in both the PDE and PTE.

`paging_is_user_accessible(virtual, require_writable)` verifies that the page
containing `virtual` is present and user-accessible. User access requires the
U/S bit in both its PDE and PTE. When `require_writable` is true, both entries
must also be writable.

`paging_is_user_range_accessible(virtual, length, require_writable)` applies
the same check to every page touched by a byte range. An empty range is valid;
a range whose final byte would overflow the 32-bit address space is rejected.
This is the required validation step before a syscall consumes a user-supplied
pointer and byte count.

Page-table access currently assumes paging structures remain in the first
identity-mapped 4 MiB. Empty page tables are not reclaimed after unmapping.

The boot test creates a temporary address space, activates it, verifies CR3,
restores the original kernel directory, frees the temporary directory, and
checks that the PMM free-frame count returns to its original value. It proves
that the shared kernel mappings are sufficient for a CR3 switch without
leaking the test frame.
