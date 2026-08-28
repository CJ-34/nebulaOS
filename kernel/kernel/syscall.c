#include <kernel/log.h>
#include <kernel/syscall.h>

#include <kernel/drivers/serial.h>
#include <kernel/paging.h>

#define SYSCALL_WRITE_MAX_LENGTH 128u

void syscall_handler(struct syscall_frame *frame)
{
    switch (frame->eax)
    {
    case SYSCALL_TEST:
        log_info("Test syscall entered from user mode\n");
        log_info("Test syscall: argument=%x\n", frame->ebx);
        break;

    case SYSCALL_ECHO:
        frame->eax = frame->ebx;
        break;

    case SYSCALL_WRITE:
        if (frame->ecx > SYSCALL_WRITE_MAX_LENGTH || !paging_is_user_range_accessible(frame->ebx, frame->ecx, false)) {
            log_error("Invalid user write: address=%x length=%x\n", frame->ebx, frame->ecx);
            frame->eax = SYSCALL_ERROR_INVALID_ARGUMENT;
            break;
        }

        serial_write_buffer((const char *)frame->ebx, frame->ecx);
        frame->eax = frame->ecx;
        break;
    default:
        log_error("Unknown syscall number: %x\n", frame->eax);
        break;
    }
}
