#include <stdint.h>

#include <kernel/gdt.h>
#include <kernel/idt.h>

static struct idt_entry idt_entries[IDT_ENTRY_COUNT];
static struct idt_pointer idt_ptr;

extern void isr_exception_0(void);
extern void isr_exception_6(void);

static void idt_set_gate(
    uint8_t vector,
    uint32_t handler_address,
    uint16_t selector,
    uint8_t type_attributes
) {
    struct idt_entry* entry = &idt_entries[vector];

    entry->offset_low = (uint16_t)(handler_address & 0xFFFF);
    entry->selector = selector;
    entry->zero = 0;
    entry->type_attributes = type_attributes;
    entry->offset_high = (uint16_t)((handler_address >> 16) & 0xFFFF);
}

void idt_init(void) {
    idt_ptr.limit = sizeof(idt_entries) - 1;
    idt_ptr.base = (uint32_t)idt_entries;

    idt_set_gate(0, (uint32_t)isr_exception_0, GDT_KERNEL_CODE_SELECTOR, 0x8E);
    idt_set_gate(6, (uint32_t)isr_exception_6, GDT_KERNEL_CODE_SELECTOR, 0x8E);

    idt_load(&idt_ptr);

}

bool idt_is_loaded(void) {
    struct idt_pointer current_ptr;

    __asm__ volatile ("sidt %0" : "=m"(current_ptr));

    return current_ptr.base == idt_ptr.base && current_ptr.limit == idt_ptr.limit;
}