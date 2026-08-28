#include <kernel/log.h>
#include <kernel/syscall.h>

#include <kernel/pit.h>

#include <kernel/drivers/serial.h>
#include <kernel/usercopy.h>

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
        char buffer[SYSCALL_WRITE_MAX_LENGTH];
        if (frame->ecx > SYSCALL_WRITE_MAX_LENGTH || !copy_from_user(buffer, frame->ebx, frame->ecx)) {
            log_error("Invalid user write: address=%x length=%x\n", frame->ebx, frame->ecx);
            frame->eax = SYSCALL_ERROR_INVALID_ARGUMENT;
            break;
        }

        serial_write_buffer(buffer, frame->ecx);
        frame->eax = frame->ecx;
        break;

    case SYSCALL_GET_TICKS:
        frame->eax = pit_ticks();
        break;
    default:
        log_error("Unknown syscall number: %x\n", frame->eax);
        break;
    }
}
