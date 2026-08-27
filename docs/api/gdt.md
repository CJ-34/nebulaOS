# GDT and TSS API

Header: `kernel/include/kernel/gdt.h`

`gdt_init()` installs the flat kernel and user segments, plus the 32-bit TSS.
The selectors are `0x08`/`0x10` for kernel code/data, `0x1B`/`0x23` for user
code/data, and `0x28` for the TSS.

`gdt_is_loaded()` verifies that `GDTR` points to NebulaOS's GDT.
`tss_load(selector)` executes `ltr`; `gdt_is_tss_loaded()` reads `TR` to
verify the TSS selector. The TSS supplies the ring-0 stack for privilege
changes and is not used for hardware task switching.
