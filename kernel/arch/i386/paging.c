#include <string.h>

#include <kernel/paging.h>
#include <kernel/pmm.h>

#define PAGING_ENTRIES 1024

#define PAGE_PRESENT 0x001
#define PAGE_WRITABLE 0x002

#define PAGE_FRAME_MASK 0xFFFFF000u

static uint32_t page_directory_physical;
static uint32_t first_page_table_physical;




bool paging_prepare_identity_map(void)
{
    uint32_t *page_directory;
    uint32_t *first_page_table;

    if (page_directory_physical != PMM_INVALID_FRAME)
    {
        return true;
    }

    page_directory_physical = pmm_allocate_frame();
    first_page_table_physical = pmm_allocate_frame();

    if (page_directory_physical == PMM_INVALID_FRAME || first_page_table_physical == PMM_INVALID_FRAME)
    {
        if (page_directory_physical != PMM_INVALID_FRAME)
        {
            pmm_free_frame(page_directory_physical);
        }

        page_directory_physical = PMM_INVALID_FRAME;
        first_page_table_physical = PMM_INVALID_FRAME;
        return false;
    }

    page_directory = (uint32_t *)page_directory_physical;
    first_page_table = (uint32_t *)first_page_table_physical;

    memset(page_directory, 0, PMM_PAGE_SIZE);
    memset(first_page_table, 0, PMM_PAGE_SIZE);

    for (uint32_t i = 0; i < PAGING_ENTRIES; i++)
    {
        first_page_table[i] = (i * PMM_PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;
    }

    page_directory[0] = first_page_table_physical | PAGE_PRESENT | PAGE_WRITABLE;

    return true;
}

uint32_t paging_directory_physical(void)
{
    return page_directory_physical;
}

uint32_t paging_first_table_physical(void)
{
    return first_page_table_physical;
}

bool paging_enable(void)
{
    uint32_t cr0;

    if (page_directory_physical == PMM_INVALID_FRAME)
    {
        return false;
    }

    __asm__ volatile(
        "movl %0, %%cr3"
        :
        : "r"(page_directory_physical)
        : "memory");

    __asm__ volatile(
        "movl %%cr0, %0"
        : "=r"(cr0));

    cr0 |= 0x80000000u;

    __asm__ volatile(
        "movl %0, %%cr0"
        :
        : "r"(cr0)
        : "memory");

    return true;
}

bool paging_is_enabled(void)
{
    uint32_t cr0;

    __asm__ volatile(
        "movl %%cr0, %0"
        : "=r"(cr0));

    return (cr0 & 0x80000000u) != 0;
}

static void paging_invalidate(uint32_t virtual_address)
{
    if (paging_is_enabled())
    {
        __asm__ volatile(
            "invlpg (%0)"
            :
            : "r"(virtual_address)
            : "memory");
    }
}

bool paging_map_page(uint32_t virtual_address, uint32_t physical_address, uint32_t flags)
{
    uint32_t *page_directory;
    uint32_t *page_table;
    uint32_t directory_index;
    uint32_t table_index;
    uint32_t page_table_physical;

    if ((virtual_address & (PMM_PAGE_SIZE - 1)) != 0 ||
        (physical_address & (PMM_PAGE_SIZE - 1)) != 0 ||
        page_directory_physical == PMM_INVALID_FRAME)
    {
        return false;
    }

    page_directory = (uint32_t*)page_directory_physical;
    directory_index = virtual_address >> 22;
    table_index = (virtual_address >> 12) & 0x3FF;

    if (!(page_directory[directory_index] & PAGE_PRESENT))
    {
        page_table_physical = pmm_allocate_frame();

        if (page_table_physical == PMM_INVALID_FRAME)
        {
            return false;
        }

        page_table = (uint32_t *)page_table_physical;
        memset(page_table, 0, PMM_PAGE_SIZE);

        page_directory[directory_index] = page_table_physical | PAGE_PRESENT | PAGE_WRITABLE;
    }
    else
    {
        page_table_physical =
            page_directory[directory_index] & PAGE_FRAME_MASK;
        page_table = (uint32_t *)page_table_physical;
    }

    if (page_table[table_index] & PAGE_PRESENT)
    {
        return false;
    }

    page_table[table_index] = physical_address | PAGE_PRESENT | (flags & PAGING_PAGE_WRITABLE);

    paging_invalidate(virtual_address);
    return true;
}

bool paging_unmap_page(uint32_t virtual_address)
{
    uint32_t *page_directory;
    uint32_t *page_table;
    uint32_t directory_index;
    uint32_t table_index;
    uint32_t page_table_physical;

    if ((virtual_address & (PMM_PAGE_SIZE - 1)) != 0 || page_directory_physical == PMM_INVALID_FRAME)
    {
        return false;
    }

    page_directory = (uint32_t *)page_directory_physical;
    directory_index = virtual_address >> 22;
    table_index = (virtual_address >> 12) & 0x3FF;

    if (!(page_directory[directory_index] & PAGE_PRESENT))
    {
        return false;
    }

    page_table_physical =
        page_directory[directory_index] & PAGE_FRAME_MASK;
    page_table = (uint32_t *)page_table_physical;

    if (!(page_table[table_index] & PAGE_PRESENT))
    {
        return false;
    }

    page_table[table_index] = 0;
    paging_invalidate(virtual_address);
    return true;
}