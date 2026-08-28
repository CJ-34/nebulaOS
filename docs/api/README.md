# NebulaOS kernel API reference

Each public kernel header has a matching reference page. These pages describe
the current contract and limitations; they are not promises of a stable
userspace ABI.

| Header | Reference |
| --- | --- |
| `assert.h` | [Assertions](assert.md) |
| `console.h` | [Console](console.md) |
| `exceptions.h` | [Exceptions](exceptions.md) |
| `gdt.h` | [GDT and TSS](gdt.md) |
| `heap.h` | [Kernel heap](heap.md) |
| `idt.h` | [IDT](idt.md) |
| `keyboard.h` | [Keyboard](keyboard.md) |
| `log.h` | [Logging](log.md) |
| `paging.h` | [Paging](paging.md) |
| `panic.h` | [Panic](panic.md) |
| `pic.h` | [PIC](pic.md) |
| `pit.h` | [PIT](pit.md) |
| `pmm.h` | [Physical memory manager](pmm.md) |
| `syscall.h` | [Syscall](syscall.md) |
| `tty.h` | [Text terminal](tty.md) |
| `usercopy.h` | [User-copy](usercopy.md) |
| `user_mode.h` | [User mode](user-mode.md) |
| `vga.h` | [VGA helpers](vga.md) |

For subsystem relationships and the boot sequence, see the
[kernel architecture](../kernel-architecture.md).
