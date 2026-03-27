#include <stdio.h>

#include <kernel/tty.h>
#include <kernel/vga.h>
#include <kernel/serial.h>

void kernel_main(void) {
  serial_init(); 
  
  if (serial_is_initialized()) {
    serial_write_string("[serial] initialized\n");
    serial_write_string("[kernel] hello from kernel\n");
  }

  terminal_initialize();

  printf("Welcome to ");
  terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
  printf("nebulaOS!\n");
}
