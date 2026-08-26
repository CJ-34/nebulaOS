#include <kernel/log.h>
#include <kernel/syscall.h>

void syscall_handler(void)
{
    log_info("Syscall entered from user mode\n");
}
