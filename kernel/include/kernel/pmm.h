#ifndef _KERNEL_PMM_H
#define _KERNEL_PMM_H

#include <stdbool.h>
#include <stdint.h>

#include <i386/multiboot.h>

#define PMM_PAGE_SIZE 4096u
#define PMM_INVALID_FRAME 0u

bool pmm_init(const multiboot_info_t* mbi);

uint32_t pmm_total_frames(void);
uint32_t pmm_free_frames(void);

uint32_t pmm_allocate_frame(void);
bool pmm_free_frame(uint32_t physical_address);

#endif