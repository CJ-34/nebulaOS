#ifndef _KERNEL_PAGING_H
#define _KERNEL_PAGING_H

#include <stdbool.h>
#include <stdint.h>

#define PAGING_PAGE_WRITABLE 0x002u
#define PAGING_PAGE_USER 0x004u

bool paging_map_page(
    uint32_t virtual_address,
    uint32_t physical_address,
    uint32_t flags
);

bool paging_unmap_page(uint32_t virtual_address);

bool paging_prepare_identity_map(void);

uint32_t paging_directory_physical(void);
uint32_t paging_first_table_physical(void);
uint32_t paging_create_address_space(void);
uint32_t paging_active_directory_physical(void);

bool paging_enable(void);
bool paging_is_enabled(void);
bool paging_is_user_accessible(uint32_t virtual_address, bool require_writable);
bool paging_is_user_range_accessible(uint32_t virtual_address, uint32_t length, bool require_writable);
bool paging_activate_address_space(uint32_t directory_physical);


#endif