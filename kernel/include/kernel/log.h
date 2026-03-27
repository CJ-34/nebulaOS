#ifndef _KERNEL_LOG_H
#define _KERNEL_LOG_H

#include <kernel/drivers/serial.h>

typedef enum log_level {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARM,
    LOG_ERROR,
} log_level_t;

void log_init(void);

void log_write(log_level_t level, const char* module, const char* msg);

#endif