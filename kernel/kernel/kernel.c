#include <stdio.h>

#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pic.h>
#include <kernel/pit.h>
#include <kernel/pmm.h>
#include <kernel/paging.h>

#include <kernel/tty.h>
#include <kernel/vga.h>
#include <kernel/log.h>

#include <kernel/panic.h>
#include <kernel/assert.h>

#include <i386/multiboot.h>

extern char __kernel_start[];
extern char __kernel_end[];

static const char *memory_type_name(uint32_t type)
{
  return type == 1 ? "available" : "reserved";
}

static void log_memory_map(const multiboot_info_t *mbi)
{
  if (!(mbi->flags & MULTIBOOT_INFO_MEM_MAP))
  {
    log_error("Multiboot memory map is unavailable\n");
    return;
  }

  uint32_t current = mbi->mmap_addr;
  uint32_t end = mbi->mmap_addr + mbi->mmap_length;

  log_info("Multiboot memory map:\n");

  while (current < end)
  {
    const struct multiboot_mmap_entry *entry =
        (const struct multiboot_mmap_entry *)current;

    uint32_t entry_total_size = entry->size + sizeof(entry->size);

    if (entry->size < 20 || entry_total_size > end - current)
    {
      log_error("Invalid Multiboot memory-map entry\n");
      return;
    }

    log_info(
        " base=%x:%x length=%x:%x type=%d (%s)\n",
        (uint32_t)(entry->addr >> 32),
        (uint32_t)entry->addr,
        (uint32_t)(entry->len >> 32),
        (uint32_t)entry->len,
        entry->type,
        memory_type_name(entry->type));

    current += entry_total_size;
  }
}

void kernel_main(uint32_t magic, multiboot_info_t *mbi)
{
  if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
  {
    PANICF("Not loading from a multiboot header");
    return;
  }

  gdt_init();

  if (!gdt_is_loaded())
  {
    PANIC("GDT was not loaded");
  }

  log_init();

  log_info("GDT loaded successfully\n");

  log_info(
      "Kernel physical range: %x - %x\n",
      (uint32_t)__kernel_start,
      (uint32_t)__kernel_end);

  log_memory_map(mbi);

  if (!pmm_init(mbi))
  {
    PANIC("Could not initialize physical memory manager");
  }

  log_info(
      "PMM: tracking %d frames, %d free frames\n",
      pmm_total_frames(),
      pmm_free_frames());

  uint32_t free_before = pmm_free_frames();
  uint32_t frame_a = pmm_allocate_frame();
  uint32_t frame_b = pmm_allocate_frame();

  ASSERT(frame_a != PMM_INVALID_FRAME);
  ASSERT(frame_b != PMM_INVALID_FRAME);
  ASSERT(frame_a != frame_b);
  ASSERT(pmm_free_frames() == free_before - 2);

  log_info("PMM allocated frames: %x and %x\n", frame_a, frame_b);

  ASSERT(pmm_free_frame(frame_a));
  ASSERT(pmm_free_frame(frame_b));
  ASSERT(pmm_free_frames() == free_before);

  log_info("PMM allocation test passed\n");

  idt_init();

  if (!idt_is_loaded())
  {
    PANIC("IDT was not loaded");
  }

  log_info("IDT loaded successfully\n");

  if (!paging_prepare_identity_map())
  {
    PANIC("Could not create identity page map");
  }

  if (!paging_enable())
  {
    PANIC("Could not enable paging");
  }

  if (!paging_is_enabled())
  {
    PANIC("Paging bit is not set");
  }

  log_info(
      "Paging structures ready: PD=%x PT=%x\n",
      paging_directory_physical(),
      paging_first_table_physical());

  log_info("Paging enabled with first 4 MiB identity-mapped\n");

  pic_init();

  log_info("PIC remapped; all IRQs masked\n");

  pit_init(100);

  log_info("Timer IRQ path installed; PIC still masked\n");

  pic_unmask_irq(0);

  __asm__ volatile("sti" ::: "memory");

  log_info("Timer IRQ enabled\n");

  uint32_t start_ticks = pit_ticks();

  while (pit_ticks() - start_ticks < 100)
  {
    __asm__ volatile("hlt");
  }

  __asm__ volatile("cli" ::: "memory");

  log_info(
      "Timer test passed: %d ticks\n",
      pit_ticks() - start_ticks);

  // __asm__ volatile (
  //   "xorl %%edx, %%edx\n\t"
  //   "movl $1, %%eax\n\t"
  //   "divl %%edx"
  //   :
  //   :
  //   :"eax", "edx", "cc"
  // );

  // PANIC("Divide-by-zero test unexpectedly returned");

  // log_info("Triggering invalid-opcode test\n");

  // __asm__ volatile ("ud2");

  // PANIC("Invalid-opcode test unexpectedly returned");

  // log_info("Triggering general-protection-fault test\n");

  // __asm__ volatile (
  //   "movw $0x18, %%ax\n\t"
  //   "movw %%ax, %%ds"
  //   :
  //   :
  //   : "ax", "memory"
  // );

  // PANIC("General-protection-fault test unexpectedly returned");

  uint16_t cs;
  uint16_t ds;
  uint16_t ss;

  __asm__ volatile("movw %%cs, %0" : "=r"(cs));
  __asm__ volatile("movw %%ds, %0" : "=r"(ds));
  __asm__ volatile("movw %%ss, %0" : "=r"(ss));

  log_info("cs == %x\n", cs);
  log_info("ds == %x\n", ds);
  log_info("ss == %x\n", ss);

  // PANIC("Kernel reached an unrecoverable state");
  // ASSERT(1 != 1);

  terminal_initialize();

  printf("Welcome to ");
  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
  printf("nebulaOS!\n");
}
