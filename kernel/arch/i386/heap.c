#include <stddef.h>
#include <stdint.h>

#include <kernel/heap.h>
#include <kernel/paging.h>
#include <kernel/pmm.h>

#define KERNEL_HEAP_START 0x00400000u
#define HEAP_ALIGNMENT 8u

struct heap_block
{
    uint32_t size;
    struct heap_block *next;
};

#define HEAP_HEADER_SIZE                                \
    ((sizeof(struct heap_block) + HEAP_ALIGNMENT - 1) & \
     ~(HEAP_ALIGNMENT - 1))

static bool heap_initialized;
static uint32_t heap_next;
static uint32_t heap_mapped_end;
static struct heap_block *free_list;

static struct heap_block *take_free_block(uint32_t size)
{
    struct heap_block *previous = NULL;
    struct heap_block *block = free_list;

    while (block != NULL)
    {
        if (block->size >= size)
        {

            uint32_t remaining_size = block->size - size;
            if (remaining_size >= HEAP_HEADER_SIZE + HEAP_ALIGNMENT)
            {
                struct heap_block* remainder = (struct heap_block*)(
                    (uint8_t*)block + HEAP_HEADER_SIZE + size
                );

                remainder->size = remaining_size - HEAP_HEADER_SIZE;
                remainder->next = block->next;

                if (previous == NULL)
                {
                    free_list = remainder;
                }
                else
                {
                    previous->next = remainder;
                }

                block->size = size;
            }
            else
            {
                if (previous == NULL)
                {
                    free_list = block->next;
                }
                else
                {
                    previous->next = block->next;
                }
            }

            return block;
        }

        previous = block;
        block = block->next;
    }

    return NULL;
}

bool heap_init(void)
{
    if (heap_initialized)
    {
        return true;
    }

    if (!paging_is_enabled())
    {
        return false;
    }

    heap_next = KERNEL_HEAP_START;
    heap_mapped_end = KERNEL_HEAP_START;
    free_list = NULL;
    heap_initialized = true;

    return true;
}

void *kmalloc(size_t size)
{
    uint32_t allocation_size;
    uint32_t allocation_address;
    uint32_t allocation_end;
    struct heap_block *block;

    if (!heap_initialized || size == 0 || size > UINT32_MAX)
    {
        return NULL;
    }

    allocation_size = (uint32_t)size;

    if (allocation_size > UINT32_MAX - (HEAP_ALIGNMENT - 1))
    {
        return NULL;
    }

    allocation_size =
        (allocation_size + HEAP_ALIGNMENT - 1) &
        ~(HEAP_ALIGNMENT - 1);

    block = take_free_block(allocation_size);

    if (block != NULL)
    {
        return (uint8_t *)block + HEAP_HEADER_SIZE;
    }

    if (allocation_size > UINT32_MAX - HEAP_HEADER_SIZE)
    {
        return NULL;
    }

    allocation_address = heap_next;
    allocation_end =
        heap_next + HEAP_HEADER_SIZE + allocation_size;

    if (allocation_end < heap_next)
    {
        return NULL;
    }

    while (allocation_end > heap_mapped_end)
    {
        uint32_t physical_frame = pmm_allocate_frame();

        if (physical_frame == PMM_INVALID_FRAME)
        {
            return NULL;
        }

        if (!paging_map_page(
                heap_mapped_end,
                physical_frame,
                PAGING_PAGE_WRITABLE))
        {
            pmm_free_frame(physical_frame);
            return NULL;
        }

        heap_mapped_end += PMM_PAGE_SIZE;
    }

    block = (struct heap_block *)allocation_address;
    block->size = allocation_size;
    block->next = NULL;

    heap_next = allocation_end;

    return (uint8_t *)block + HEAP_HEADER_SIZE;
}

static bool blocks_are_adjacent(
    const struct heap_block* first,
    const struct heap_block* second
) {
    return (uint32_t)first + HEAP_HEADER_SIZE + first->size == (uint32_t)second;
}

static void merge_with_next(struct heap_block* block) {
    struct heap_block* next = block->next;

    if (next != NULL && blocks_are_adjacent(block, next)) {
        block->size += HEAP_HEADER_SIZE + next->size;
        block->next = next->next;
    }
}

void kfree(void *pointer)
{
    struct heap_block *block;
    struct heap_block *previous = NULL;
    struct heap_block *current = free_list;

    if (pointer == NULL)
    {
        return;
    }

    block = (struct heap_block *)((uint8_t *)pointer - HEAP_HEADER_SIZE);

    while(current != NULL && (uint32_t)current < (uint32_t)block) {
        previous = current;
        current = current->next;
    }

    block->next = current;

    if (previous == NULL) {
        free_list = block;
    } else {
        previous->next = block;
    }

    merge_with_next(block);

    if (previous != NULL) {
        merge_with_next(previous);
    }
}

uint32_t heap_used_bytes(void)
{
    return heap_next - KERNEL_HEAP_START;
}

uint32_t heap_mapped_pages(void)
{
    return (heap_mapped_end - KERNEL_HEAP_START) / PMM_PAGE_SIZE;
}
