#include <kernel/log.h>
#include <kernel/syscall.h>

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

    default:
        log_error("Unknown syscall number: %x\n", frame->eax);
        break;
    }
}
