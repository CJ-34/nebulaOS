#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/vga.h>

void kernel_main(void) {
  terminal_initialize();

  printf("Welcome to ");
  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
  printf("nebulaOS!\n");
}
