# NebulaOS

NebulaOS is a learning-oriented, 32-bit i686 operating-system kernel written
in C and x86 assembly. It boots through GRUB using Multiboot 1 and is built
with a freestanding `i686-elf` cross toolchain.

## Current capabilities

- GRUB/Multiboot boot path and a linker-defined kernel image at `0x00100000`
- Flat ring-0 GDT with owned code and data segments
- IDT handlers for divide error, invalid opcode, general-protection faults,
  and page faults
- COM1 serial logging, VGA text output, kernel panic, and assertions
- Remapped PIC and a 100 Hz PIT timer IRQ
- Multiboot physical-memory-map parsing
- Bitmap physical-memory manager for 4 KiB page frames
- Initial paging setup: the first 4 MiB is identity-mapped
- Kernel heap with 8-byte-aligned allocations and free-list reuse

## Requirements

- `i686-elf-gcc`, `i686-elf-ar`, and the corresponding binutils
- GRUB tools: `grub-file` and `grub-mkrescue`
- `qemu-system-i386`

## Build and run

From the repository root:

```sh
./qemu.sh
```

The kernel writes diagnostics to COM1, and `qemu.sh` connects that serial port
to the host terminal. In a headless environment, run the generated ISO with:

```sh
qemu-system-i386 -cdrom build/nebula.iso -serial stdio -display none
```

## Project layout

```text
kernel/arch/i386/  x86 boot, descriptor tables, interrupts, PIC, PIT, paging
kernel/kernel/     architecture-independent kernel code and PMM
kernel/include/    kernel interfaces
libc/              freestanding C support library (libk)
docs/              design and architecture notes
```

## Documentation

See [Kernel architecture](docs/kernel-architecture.md) for the boot sequence,
CPU protection state, interrupt flow, memory layout, and current limitations.
See the [kernel API reference](docs/api/README.md) for one focused document per
public kernel interface.

## Near-term direction

The next user-mode milestone is a syscall register frame, followed by saved
execution contexts and scheduling so a user process can return or exit cleanly.
