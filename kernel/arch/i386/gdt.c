#include <stdint.h>
#include <kernel/gdt.h>

#include <i386/tss.h>

#define GDT_ENTRY_COUNT 6

extern char stack_top[];

static struct gdt_entry gdt_entries[GDT_ENTRY_COUNT];
static struct gdt_pointer gdt_ptr;
static struct tss_entry tss;

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
  gdt_set_entry(&gdt_entries[3], 0, 0xFFFFF, 0xFA, 0xC0);
  gdt_set_entry(&gdt_entries[4], 0, 0xFFFFF, 0xF2, 0xC0);

  tss = (struct tss_entry) {0};
  tss.esp0 = (uint32_t)stack_top;
  tss.ss0 = GDT_KERNEL_DATA_SELECTOR;
  tss.iomap_base = sizeof(tss);

  gdt_set_entry(&gdt_entries[5], (uint32_t)&tss, sizeof(tss) - 1, 0x89, 0x00);

  gdt_ptr.limit = sizeof(gdt_entries) - 1;
  gdt_ptr.base = (uint32_t)gdt_entries;

  gdt_load(&gdt_ptr);
  tss_load(GDT_TSS_SELECTOR);
}

bool gdt_is_loaded(void) {
  struct gdt_pointer current_ptr;

  __asm__ volatile ("sgdt %0" : "=m"(current_ptr));

  return current_ptr.base == gdt_ptr.base && current_ptr.limit == gdt_ptr.limit;
}

bool gdt_is_tss_loaded(void)
{
    uint16_t selector;

    __asm__ volatile("str %0" : "=r"(selector));

    return selector == GDT_TSS_SELECTOR;
}
