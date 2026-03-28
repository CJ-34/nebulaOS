#include <kernel/log.h>
#include <kernel/drivers/serial.h>
#include <stdint.h>
#include <stdarg.h>

#define LOG_MIN_LEVEL LOG_DEBUG

static const char* log_level_str(log_level_t level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARM: return "WARM";
        case LOG_ERROR: return "ERROR";
        case LOG_FATAL: return "FATAL";
        default: return "UNKNOWN";
    };
}

static void log_putchar(char c) {
    serial_write_char(c);
}

static void log_putstr(const char* str) {
    if (!str) {
        str = "(null)";
    }

    while (*str) {
        log_putchar(*str++);
    }
}

static void log_putint(int value) {
    char buffer[12];
    int i = 0;

    if (value == 0) {
        log_putchar('0');
        return;
    }

    if (value < 0) {
        log_putchar('-');
        value = -value;
    }

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        log_putchar(buffer[--i]);
    }
}

static void log_puthex(uint32_t value) {
    const char* hex = "0123456789ABCDEF";
    log_putstr("0x");

    for (int shift = 28; shift >= 0; shift -= 4) {
        log_putchar(hex[(value >> shift) & 0xF]);
    }
}

static void log_vwrite(const char* fmt, va_list args) {
    for (const char* p = fmt; *p; ++p) {
        if (*p != '%') {
            log_putchar(*p);
            continue;
        }

        ++p;

        switch (*p) {
            case 'c':
                log_putchar((char)va_arg(args, int));
                break;
            
            case 's':
                log_putstr(va_arg(args, const char*));
                break;
            
            case 'd':
                log_putint(va_arg(args, int));
                break;
            
            case 'x':
                log_puthex(va_arg(args, uint32_t));
                break;
            
            case '%':
                log_putchar('%');
                break;

            default:
                log_putchar('%');
                log_putchar(*p);
                break;
        
        }
    }
}

void log_init(void)
{
    serial_init();
}

void log_write(log_level_t level, const char* fmt, ...)
{
    if (level < LOG_MIN_LEVEL) {
        return;
    }

    va_list args;
    va_start(args, fmt);

    log_putchar('[');
    log_putstr(log_level_str(level));
    log_putchar(']');
    log_putchar(' ');

    log_vwrite(fmt, args);

    va_end(args); 
}
