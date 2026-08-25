#include <stdint.h>
#include <kernel/gdt.h>

#define GDT_ENTRY_COUNT 3

static struct gdt_entry gdt_entries[GDT_ENTRY_COUNT];
static struct gdt_pointer gdt_ptr;

static void gdt_set_entry(
  struct gdt_entry* entry,
  uint32_t base,
  uint32_t limit,
  uint8_t access,
  uint8_t flags
    ) {
  entry->limit_low = (uint16_t)(limit & 0xFFFF);
  entry->base_low = (uint16_t)(base & 0xFFFF);
  entry->base_middle = (uint8_t)((base >> 16) & 0xFF);
  entry->access = access;
  entry->granularity = 
    (uint8_t)(((limit >> 16) & 0x0F) | (flags & 0xF0));
  entry->base_high = (uint8_t)((base >> 24) & 0xFF);
}

void gdt_init(void) {
  gdt_entries[0] = (struct gdt_entry) { 0 };

  gdt_set_entry(&gdt_entries[1], 0, 0xFFFFF, 0x9A, 0xC0);
  gdt_set_entry(&gdt_entries[2], 0, 0xFFFFF, 0x92, 0xC0);

  gdt_ptr.limit = sizeof(gdt_entries) - 1;
  gdt_ptr.base = (uint32_t)gdt_entries;

  gdt_load(&gdt_ptr);
}

bool gdt_is_loaded(void) {
  struct gdt_pointer current_ptr;

  __asm__ volatile ("sgdt %0" : "=m"(current_ptr));

  return current_ptr.base == gdt_ptr.base && current_ptr.limit == gdt_ptr.limit;
}
