# NebulaOS kernel architecture

NebulaOS is a 32-bit i686 learning kernel booted by GRUB through the
Multiboot 1 protocol. It currently uses a flat kernel address space and a
small identity-mapped paging setup.

## Build and boot

The project expects an `i686-elf` cross toolchain. The normal workflow is:

```sh
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
   paging structures, PIT timer, PS/2 keyboard IRQ path, and text console.

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
| 33 | PS/2 keyboard IRQ1 | No |

No-error-code exception stubs push a synthetic zero error code. Error-code
stubs push only the vector because the CPU has already pushed the real error
code. Both paths then use the common exception frame and C panic dispatcher.

## Hardware interrupts

The legacy 8259 PIC is remapped so IRQs 0--15 use IDT vectors 32--47. All
IRQs start masked. The kernel unmasks IRQ0 and IRQ1 only after their IDT gates
and handlers are ready.

The PIT is programmed for approximately 100 Hz. Its handler increments a
volatile tick counter and sends an EOI to the PIC. The current boot test
enables interrupts, waits with `hlt` for 100 ticks, and logs the result. IRQs
remain enabled for the interactive console that follows.

The PS/2 keyboard driver reads one scan code from port `0x60` on IRQ1, places
it in a 64-byte ring buffer, and immediately sends an EOI. The main kernel
loop consumes that buffer and translates basic Set-1 make codes into ASCII.
It handles left/right Shift, Enter, Backspace, and printable US-layout keys;
extended and key-release codes are ignored after Shift state is updated.

## Kernel console

After boot tests complete, NebulaOS initializes the VGA text terminal and
enters an interrupt-driven console loop. The loop drains keyboard characters,
edits a bounded 64-character input line, and halts with `hlt` while no input
is pending. This avoids busy-waiting and lets hardware interrupts wake the CPU.

Current commands are:

| Command | Purpose |
| --- | --- |
| `help` | List console commands. |
| `clear` | Clear and reinitialize the VGA terminal. |
| `mem` | Print PMM total and free frame counts. |
| `heap` | Print heap high-water usage and mapped-page count. |
| `heaptest` | Exercise released-block reuse and writable heap memory. |
| `ticks` | Print the PIT interrupt count. |
| `panic` | Deliberately invoke the kernel panic path. |

`heaptest` allocates a 256-byte block, records the heap high-water mark, frees
the block, then requests 64 bytes. It verifies that the high-water mark does
not increase—which means the allocator used an existing free block rather than
extending the heap—and verifies writes at offsets 0 and 63 of the returned
payload. It frees the test allocation before returning to the console.

## VGA text terminal

The terminal writes directly to the VGA text buffer at physical address
`0xB8000`. Each 16-bit cell contains an ASCII character and its foreground and
background color. When a newline or a character at column 79 would advance the
cursor beyond row 24, the terminal scrolls: rows 1--24 are copied to rows
0--23, row 24 is cleared using the current color, and the cursor remains on
the final row. This replaces the earlier behavior that wrapped to row 0 and
overwrote the oldest output.

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

## Kernel heap

The heap begins at virtual address `0x00400000`, immediately above the initial
identity map. It obtains physical frames from the PMM and maps each new heap
page through `paging_map_page()` as the heap grows.

`kmalloc()` returns 8-byte-aligned payload addresses. Each allocation has a
small header directly before its payload:

```text
[ payload size | free-list next pointer ][ caller payload ]
```

`kfree()` recovers that header from the payload pointer and inserts the block
into an address-sorted singly linked free list. `kmalloc()` searches that list
before extending the heap, so a released block can be reused without mapping a
new physical frame. An oversized free block is split when the remainder can
hold another header and aligned payload; adjacent free blocks are coalesced on
free.

The current heap is intentionally simple: it does not shrink mapped pages,
validate pointers, detect double-frees, or synchronize concurrent callers.

## Current limits and next work

- Only the first 4 MiB is mapped after paging is enabled.
- The kernel uses one flat ring-0 address space; there is no userspace or TSS.
- The exception dispatcher reports page faults and reads `CR2`, but recovery
  policy is not implemented.
- IRQ0 and IRQ1 are the only unmasked hardware interrupts.
- PMM allocation is a linear bitmap scan and has no locking for concurrent
  allocation.
- Heap free-list operations are not yet synchronized and have no fragmentation
  control.

The next useful console milestone is command arguments or a dedicated
diagnostic interface for inspecting allocator state. Longer-term input work
includes modifier locking, extended keys, and a keyboard layout abstraction.
