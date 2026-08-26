#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include <stdint.h>
#include <stdbool.h>

#define SYSCALL_VECTOR 0x80
#define IDT_ENTRY_COUNT 256

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attributes;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_pointer {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void idt_init(void);

void idt_load(const struct idt_pointer *idt_ptr);
bool idt_is_loaded(void);

#endif