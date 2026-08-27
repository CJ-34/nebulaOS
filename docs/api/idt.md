# IDT API

Header: `kernel/include/kernel/idt.h`

`idt_init()` installs the current exception, timer, keyboard, and syscall
gates. `idt_is_loaded()` verifies that `IDTR` points to NebulaOS's table.

`SYSCALL_VECTOR` is `0x80`. Its gate has DPL 3 so ring-3 code may execute
`int $0x80`; the exception and hardware-IRQ gates remain ring-0-only.
