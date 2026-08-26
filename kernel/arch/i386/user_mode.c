#include <kernel/paging.h>
#include <kernel/pmm.h>
#include <kernel/user_mode.h>

#include <string.h>

static const uint8_t initial_user_code[] = {
    0xCD, 0x80,
    0xEB, 0xFE
};

bool user_mode_prepare(void)
{
    uint32_t code_frame = pmm_allocate_frame();

    if (code_frame == PMM_INVALID_FRAME)
    {
        return false;
    }

    uint32_t stack_frame = pmm_allocate_frame();

    if (stack_frame == PMM_INVALID_FRAME)
    {
        pmm_free_frame(code_frame);
        return false;
    }

    memset((void *)code_frame, 0, PMM_PAGE_SIZE);
    memset((void *)stack_frame, 0, PMM_PAGE_SIZE);

    memcpy(
        (void *)code_frame,
        initial_user_code,
        sizeof(initial_user_code));

    if (!paging_map_page(
            USER_CODE_VIRTUAL,
            code_frame,
            PAGING_PAGE_USER))
    {
        pmm_free_frame(stack_frame);
        pmm_free_frame(code_frame);
        return false;
    }

    if (!paging_map_page(
            USER_STACK_VIRTUAL,
            stack_frame,
            PAGING_PAGE_USER | PAGING_PAGE_WRITABLE))
    {
        paging_unmap_page(USER_CODE_VIRTUAL);
        pmm_free_frame(stack_frame);
        pmm_free_frame(code_frame);
        return false;
    }

    return true;
}