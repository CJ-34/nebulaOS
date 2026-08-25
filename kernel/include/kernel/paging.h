#ifndef _KERNEL_PAGING_H
#define _KERNEL_PAGING_H

#include <stdbool.h>
#include <stdint.h>

bool paging_prepare_identity_map(void);

uint32_t paging_directory_physical(void);
uint32_t paging_first_table_physical(void);

bool paging_enable(void);
bool paging_is_enabled(void);

#endif