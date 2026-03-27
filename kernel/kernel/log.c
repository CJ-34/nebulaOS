#include <kernel/log.h>

void log_init(void)
{
    serial_init();

}

void log_write(log_level_t level, const char *module, const char *msg)
{
    if (!serial_is_initialized()) {
        // handle error
        return;
    }
}
