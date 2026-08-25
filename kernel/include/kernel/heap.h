#ifndef _KERNEL_HEAP_H
#define _KERNEL_HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool heap_init(void);
void *kmalloc(size_t size);
void kfree(void* pointer);

uint32_t heap_used_bytes(void);
uint32_t heap_mapped_pages(void);

#endif