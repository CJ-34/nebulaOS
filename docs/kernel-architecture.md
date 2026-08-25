# NebulaOS kernel architecture

NebulaOS is a 32-bit i686 learning kernel booted by GRUB through the
Multiboot 1 protocol. It currently uses a flat kernel address space and a
small identity-mapped paging setup.

## Build and boot

The project expects an `i686-elf` cross toolchain. The normal workflow is:

```sh
make
./iso.sh
./qemu.sh
```

`qemu.sh` routes COM1 to the host terminal with `-serial stdio`. In a
headless environment, use:

```sh
qemu-system-i386 -cdrom build/nebula.iso -serial stdio -display none
```

## Boot path

1. GRUB finds the Multiboot header in `arch/i386/boot.S` and loads the ELF
   kernel according to `arch/i386/linker.ld`.
2. The linker places the kernel at physical address `0x00100000` and exposes
   `__kernel_start` and `__kernel_end` symbols.
3. GRUB enters `_start` in 32-bit protected mode with the Multiboot magic in
   `EAX` and the Multiboot-information pointer in `EBX`.
4. `_start` selects the 16 KiB bootstrap stack and calls `kernel_main` with
   `kernel_main(magic, mbi)`.
5. The kernel installs its GDT, serial logger, IDT, physical-memory manager,
   paging structures, and timer path.

Until paging is enabled, the flat GDT means logical, linear, and physical
addresses are numerically identical.

## CPU protection state

The kernel owns a three-entry GDT:

| Selector | Descriptor |
| --- | --- |
| `0x00` | Null descriptor |
| `0x08` | Ring-0, flat 32-bit code segment |
| `0x10` | Ring-0, flat 32-bit data/stack segment |

`gdt_load.S` executes `lgdt`, uses a far jump to reload `CS`, then reloads
the data segment registers. The boot self-check confirms the active GDT and
that `CS == 0x08`, `DS == SS == 0x10`.

The IDT has handlers for:

| Vector | Exception | CPU error code |
| --- | --- | --- |
| 0 | Divide error | No |
| 6 | Invalid opcode | No |
| 13 | General-protection fault | Yes |
| 14 | Page fault | Yes |
| 32 | PIT timer IRQ0 | No |

No-error-code exception stubs push a synthetic zero error code. Error-code
stubs push only the vector because the CPU has already pushed the real error
code. Both paths then use the common exception frame and C panic dispatcher.

## Hardware interrupts

The legacy 8259 PIC is remapped so IRQs 0--15 use IDT vectors 32--47. All
IRQs start masked. The kernel unmasks only IRQ0 after the IDT gate and PIT
handler are ready.

The PIT is programmed for approximately 100 Hz. Its handler increments a
volatile tick counter and sends an EOI to the PIC. The current boot test
enables interrupts, waits with `hlt` for 100 ticks, logs the result, and then
disables interrupts again.

## Physical memory manager

GRUB's Multiboot memory map is parsed as a variable-length list; each entry
advances by `entry->size + 4`, not by `sizeof(struct multiboot_mmap_entry)`.

The PMM manages 4 KiB physical frames with a 128 KiB bitmap, enough to track
the entire 32-bit physical-address space:

```text
4 GiB / 4 KiB = 1,048,576 frames = 128 KiB bitmap
```

Initialization marks every frame used, frees only Multiboot type-1 ranges,
then reserves the first MiB, the linker-defined kernel range, the Multiboot
information structure, and the memory-map storage. `pmm_allocate_frame()`
returns a physical 4 KiB frame and marks it used before returning.

`pmm_free_frame()` must currently be used only on frames returned by the PMM;
the bitmap does not yet distinguish a dynamically allocated frame from a
permanently reserved one.

## Paging

The first paging configuration uses standard 4 KiB x86 pages:

```text
page directory: one PMM frame
first page table: one PMM frame
PDE[0] -> first page table
PTE[0..1023] -> physical frames 0..1023
```

This identity maps the first 4 MiB, including the kernel, bootstrap stack,
PMM bitmap, Multiboot data, VGA memory, GDT/IDT data, and the initial paging
structures. `paging_enable()` loads the page-directory physical address into
`CR3` and sets `CR0.PG`; `paging_is_enabled()` verifies that bit afterward.

## Current limits and next work

- Only the first 4 MiB is mapped after paging is enabled.
- The kernel uses one flat ring-0 address space; there is no userspace or TSS.
- The exception dispatcher reports page faults but does not yet read `CR2`.
- IRQ0 is the only unmasked hardware interrupt.
- PMM allocation is a linear bitmap scan and has no locking for concurrent
  allocation.

The next useful paging milestone is a general page-mapping interface, then a
controlled page-fault test and an explicit policy for mapping physical frames
above the initial 4 MiB identity map.
