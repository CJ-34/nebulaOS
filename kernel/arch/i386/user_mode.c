#include <kernel/paging.h>
#include <kernel/pmm.h>
#include <kernel/user_mode.h>
#include <kernel/syscall.h>

#include <string.h>

#define INITIAL_USER_MESSAGE_OFFSET 0x100u
#define INITIAL_USER_MESSAGE_VIRTUAL \
    (USER_CODE_VIRTUAL + INITIAL_USER_MESSAGE_OFFSET)

static const uint8_t initial_user_message[] = "Hello from ring 3\n";

#define INITIAL_USER_MESSAGE_LENGTH \
    (sizeof(initial_user_message) - 1u)

static const uint8_t initial_user_code[] = {
    /* mov $INITIAL_USER_MESSAGE_VIRTUAL, %ebx */
    0xBB, 0x00, 0x01, 0x80, 0x00,

    /* mov $INITIAL_USER_MESSAGE_LENGTH, %ecx */
    0xB9, INITIAL_USER_MESSAGE_LENGTH, 0x00, 0x00, 0x00,

    /* mov $SYSCALL_WRITE, %eax; int $0x80 */
    0xB8, SYSCALL_WRITE, 0x00, 0x00, 0x00,
    0xCD, 0x80,

    /* Preserve SYSCALL_WRITE's EAX return value as TEST's EBX argument. */
    0x89, 0xC3,

    /* mov $SYSCALL_TEST, %eax; int $0x80 */
    0xB8, SYSCALL_TEST, 0x00, 0x00, 0x00,
    0xCD, 0x80,

    /* mov $SYSCALL_GET_TICKS, %eax; int $0x80 */
    0xB8, SYSCALL_GET_TICKS, 0x00, 0x00, 0x00,
    0xCD, 0x80,

    /* mov %eax, %ebx */
    0x89, 0xC3,

    /* mov $SYSCALL_TEST, %eax; int $0x80 */
    0xB8, SYSCALL_TEST, 0x00, 0x00, 0x00,
    0xCD, 0x80,

    /* jmp . */
    0xEB, 0xFE};

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

    memcpy((uint8_t *)code_frame + INITIAL_USER_MESSAGE_OFFSET,
           initial_user_message,
           INITIAL_USER_MESSAGE_LENGTH);

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