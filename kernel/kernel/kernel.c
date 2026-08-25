#include <stdio.h>

#include <kernel/gdt.h>
#include <kernel/idt.h>

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

  gdt_init();

  if (!gdt_is_loaded()) {
    PANIC("GDT was not loaded");
  }

  
  log_init();

  log_info("GDT loaded successfully\n");

  idt_init();
 
  if (!idt_is_loaded()) {
    PANIC("IDT was not loaded");
  }

  log_info("IDT loaded successfully\n");

  // __asm__ volatile (
  //   "xorl %%edx, %%edx\n\t"
  //   "movl $1, %%eax\n\t"
  //   "divl %%edx"
  //   :
  //   :
  //   :"eax", "edx", "cc"
  // );

  // PANIC("Divide-by-zero test unexpectedly returned");

  log_info("Triggering invalid-opcode test\n");

  __asm__ volatile ("ud2");

  PANIC("Invalid-opcode test unexpectedly returned");

  uint16_t cs;
  uint16_t ds;
  uint16_t ss;

  __asm__ volatile ("movw %%cs, %0" : "=r"(cs));
  __asm__ volatile ("movw %%ds, %0" : "=r"(ds));
  __asm__ volatile ("movw %%ss, %0" : "=r"(ss));

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
