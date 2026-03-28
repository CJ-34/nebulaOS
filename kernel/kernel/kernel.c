#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/vga.h>
#include <kernel/log.h>

#include <kernel/panic.h>
#include <kernel/assert.h>

void kernel_main(void) {
  log_init();
  log_info("Nebula kernel starting...\n");
  log_debug("Testing integer: %d\n", 67);
  log_warm("This is a warning\n");
  log_error("This is an error code: %x\n", 0xAE96C);

  // PANIC("Kernel reached an unrecoverable state");
  // ASSERT(1 != 1);


  terminal_initialize();

  printf("Welcome to ");
  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
  printf("nebulaOS!\n");
}
