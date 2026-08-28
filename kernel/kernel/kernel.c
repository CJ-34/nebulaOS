#include <stdio.h>

#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pic.h>
#include <kernel/pit.h>
#include <kernel/pmm.h>
#include <kernel/paging.h>
#include <kernel/heap.h>
#include <kernel/user_mode.h>
#include <kernel/usercopy.h>
#include <kernel/task.h>

#include <kernel/tty.h>
#include <kernel/console.h>
#include <kernel/vga.h>
#include <kernel/log.h>
#include <kernel/keyboard.h>

#include <kernel/panic.h>
#include <kernel/assert.h>

#include <i386/multiboot.h>

extern char __kernel_start[];
extern char __kernel_end[];
extern char stack_top[];

#define PAGING_TEST_VIRTUAL 0x00400000u
#define PAGING_TEST_VALUE 0x4E454255u

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

static void run_console(void)
{
  terminal_initialize();

  printf("Welcome to ");
  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
  printf("nebulaOS!\n");
  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

  console_init();

  log_info("Keyboard IRQ enabled; press keys in QEMU\n");

  for (;;)
  {
    char character;

    while (keyboard_pop_char(&character))
    {
      console_handle_char(character);
    }

    __asm__ volatile("hlt");
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

  if (!gdt_is_tss_loaded())
  {
    PANIC("TSS was not loaded");
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

  struct task boot_task = {
      .id = 0,
      .state = TASK_RUNNING,
      .page_directory_physical = paging_directory_physical(),
      .kernel_stack_top = (uint32_t)stack_top,
  };

  ASSERT(boot_task.id == 0);
  ASSERT(boot_task.state == TASK_RUNNING);
  ASSERT(boot_task.page_directory_physical ==
         paging_directory_physical());
  ASSERT(boot_task.kernel_stack_top == (uint32_t)stack_top);

  log_info("Boot task model test passed\n");

  uint32_t mapping_free_before = pmm_free_frames();
  uint32_t mapped_frame = pmm_allocate_frame();

  ASSERT(mapped_frame != PMM_INVALID_FRAME);

  ASSERT(paging_map_page(PAGING_TEST_VIRTUAL, mapped_frame, PAGING_PAGE_WRITABLE));

  volatile uint32_t *mapped_page =
      (volatile uint32_t *)PAGING_TEST_VIRTUAL;

  *mapped_page = PAGING_TEST_VALUE;

  ASSERT(*mapped_page == PAGING_TEST_VALUE);

  /*
   * The allocated frame is currently below 4 MiB, so its identity mapping
   * still exists. This confirms both virtual addresses reach the same frame.
   */
  ASSERT(*(volatile uint32_t *)mapped_frame == PAGING_TEST_VALUE);

  log_info(
      "Paging mapped virtual %x to physical %x\n",
      PAGING_TEST_VIRTUAL,
      mapped_frame);

  ASSERT(paging_unmap_page(PAGING_TEST_VIRTUAL));
  ASSERT(pmm_free_frame(mapped_frame));

  /*
   * The target frame was released. The new second page table stays allocated,
   * so exactly one PMM frame remains in use.
   */
  ASSERT(pmm_free_frames() == mapping_free_before - 1);

  log_info("Paging map/unmap test passed\n");

  // log_info("Triggering controlled page fault\n");

  // volatile uint32_t fault_value =
  //     *(volatile uint32_t *)PAGING_TEST_VIRTUAL;

  // (void)fault_value;

  // PANIC("Controlled page-fault test unexpectedly returned");

  if (!heap_init())
  {
    PANIC("Could not initialize kernel heap");
  }

  uint8_t *byte = kmalloc(1);
  uint32_t *word = kmalloc(sizeof(uint32_t));
  uint8_t *large = kmalloc(PMM_PAGE_SIZE);

  ASSERT(byte != NULL);
  ASSERT(word != NULL);
  ASSERT(large != NULL);

  ASSERT(((uint32_t)byte & 7) == 0);
  ASSERT(((uint32_t)word & 7) == 0);
  ASSERT(((uint32_t)large & 7) == 0);

  *byte = 0x42;
  *word = 0x4e454255;
  large[0] = 0xAA;
  large[PMM_PAGE_SIZE - 1] = 0x55;

  ASSERT(*byte == 0x42);
  ASSERT(*word == 0x4E454255);
  ASSERT(large[0] == 0xAA);
  ASSERT(large[PMM_PAGE_SIZE - 1] == 0x55);

  log_info(
      "Heap test passed: %d bytes across %d pages\n",
      heap_used_bytes(),
      heap_mapped_pages());

  void *freed_byte_address = byte;
  kfree(byte);
  byte = NULL;

  uint8_t *reused_byte = kmalloc(1);

  ASSERT(reused_byte == freed_byte_address);

  *reused_byte = 0x24;
  ASSERT(*reused_byte == 0x24);

  log_info("Heap allocation/free test passed\n");

  uint32_t mapped_pages_before_split = heap_mapped_pages();
  void *large_address = large;

  kfree(large);
  large = NULL;

  uint8_t *small_from_large = kmalloc(64);
  uint8_t *remainder_from_large = kmalloc(4000);

  ASSERT(small_from_large == large_address);
  ASSERT(remainder_from_large != NULL);
  ASSERT(heap_mapped_pages() == mapped_pages_before_split);

  small_from_large[0] = 0x11;
  remainder_from_large[0] = 0x22;
  remainder_from_large[3999] = 0x33;

  ASSERT(small_from_large[0] == 0x11);
  ASSERT(remainder_from_large[0] == 0x22);
  ASSERT(remainder_from_large[3999] == 0x33);

  log_info("Heap block-splitting test passed\n");

  uint8_t *left = kmalloc(128);
  uint8_t *right = kmalloc(128);

  ASSERT(left != NULL);
  ASSERT(right != NULL);

  uint32_t mapped_pages_before_merge = heap_mapped_pages();
  uint32_t heap_used_before_merge = heap_used_bytes();

  kfree(left);
  kfree(right);

  left = NULL;
  right = NULL;

  uint8_t *merged = kmalloc(256);

  ASSERT(heap_used_bytes() == heap_used_before_merge);
  ASSERT(heap_mapped_pages() == mapped_pages_before_merge);

  merged[0] = 0x44;
  merged[255] = 0x55;

  ASSERT(merged[0] == 0x44);
  ASSERT(merged[255] == 0x55);

  log_info("Heap block-coalescing test passed\n");

  if (!user_mode_prepare())
  {
    PANIC("Could not prepare user-mode memory");
  }

  uint8_t copied_byte = 0;

  ASSERT(copy_from_user(
      &copied_byte,
      USER_CODE_VIRTUAL,
      sizeof(copied_byte)));

  ASSERT(copied_byte == 0xBB);

  ASSERT(!copy_from_user(
      &copied_byte,
      0x00801000u,
      sizeof(copied_byte)));

  log_info("User-copy test passed\n");

  log_info("User code and stack pages mapped\n");

  ASSERT(paging_is_user_accessible(USER_CODE_VIRTUAL, false));
  ASSERT(!paging_is_user_accessible(USER_CODE_VIRTUAL, true));

  ASSERT(paging_is_user_accessible(USER_STACK_VIRTUAL, false));
  ASSERT(paging_is_user_accessible(USER_STACK_VIRTUAL, true));

  ASSERT(!paging_is_user_accessible(0x00801000u, false));
  ASSERT(!paging_is_user_accessible(0x00100000u, false));

  log_info("User-page permission test passed\n");

  ASSERT(paging_is_user_range_accessible(
      USER_CODE_VIRTUAL + PMM_PAGE_SIZE - 16,
      16,
      false));

  ASSERT(!paging_is_user_range_accessible(
      USER_CODE_VIRTUAL + PMM_PAGE_SIZE - 16,
      17,
      false));

  ASSERT(paging_is_user_range_accessible(
      USER_STACK_VIRTUAL + PMM_PAGE_SIZE - 16,
      16,
      true));

  ASSERT(paging_is_user_range_accessible(0xFFFFFFFFu, 0, false));

  ASSERT(!paging_is_user_range_accessible(0xFFFFFFF0u, 32, false));

  log_info("User-range permission test passed\n");

  pic_init();

  log_info("PIC remapped; all IRQs masked\n");

  pit_init(100);

  log_info("Timer IRQ path installed; PIC still masked\n");

  keyboard_init();

  pic_unmask_irq(0);
  pic_unmask_irq(1);

  __asm__ volatile("sti" ::: "memory");

  log_info("Timer IRQ enabled\n");

  uint32_t start_ticks = pit_ticks();

  while (pit_ticks() - start_ticks < 100)
  {
    __asm__ volatile("hlt");
  }

  log_info(
      "Timer test passed: %d ticks\n",
      pit_ticks() - start_ticks);

  run_console();
}
