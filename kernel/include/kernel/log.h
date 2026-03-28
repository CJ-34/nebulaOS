#ifndef _KERNEL_LOG_H
#define _KERNEL_LOG_H

#define log_debug(...) log_write(LOG_DEBUG, __VA_ARGS__)
#define log_info(...) log_write(LOG_INFO, __VA_ARGS__)
#define log_warm(...) log_write(LOG_WARM, __VA_ARGS__)
#define log_error(...) log_write(LOG_ERROR, __VA_ARGS__)

typedef enum log_level {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARM,
    LOG_ERROR,
    LOG_FATAL,
} log_level_t;

void log_init(void);
void log_write(log_level_t level, const char* fmt, ...);

#endif