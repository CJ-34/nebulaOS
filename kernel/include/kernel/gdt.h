#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

#include <stdint.h>
#include <stdbool.h>

#define GDT_KERNEL_CODE_SELECTOR 0x08
#define GDT_KERNEL_DATA_SELECTOR 0x10
#define GDT_USER_CODE_SELECTOR 0x1B
#define GDT_USER_DATA_SELECTOR 0x23
#define GDT_TSS_SELECTOR 0x28

struct gdt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__((packed));

struct gdt_pointer {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

void gdt_init(void);

void gdt_load(const struct gdt_pointer* gdt_ptr);

bool gdt_is_loaded(void);

void tss_load(uint16_t selector);

bool gdt_is_tss_loaded(void);

void gdt_set_kernel_stack(uint32_t stack_top);

#endif
