#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/vga.h>
#include <kernel/log.h>

#include <kernel/panic.h>
#include <kernel/assert.h>

#include <i386/multiboot.h>

void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    PANICF("Not loading from a multiboot header");
    return;
  }
  uint32_t mem_low;
  uint32_t mem_high;
  if (mbi->flags & MULTIBOOT_INFO_MEMORY) {
    mem_low = mbi->mem_lower;
    mem_high = mbi->mem_upper;
  }
  
  log_init();
  log_info("Nebula kernel starting...\n");
  log_debug("Testing integer: %d\n", 67);
  log_warm("This is a warning\n");
  log_error("This is an error code: %x\n", 0xAE96C);

  log_info("Memory lower address: %d KB\n", mem_low);
  log_info("Memory upper address: %d KB\n", mem_high);

  // PANIC("Kernel reached an unrecoverable state");
  // ASSERT(1 != 1);

  terminal_initialize();

  printf("Welcome to ");
  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
  printf("nebulaOS!\n");
}
