#include <kernel/panic.h>
#include <kernel/drivers/serial.h>
#include <kernel/log.h>

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

static volatile int g_panic_active = 0;

static inline void arch_disable_interrupts(void) {
    __asm__ volatile ("cli");
}

static __attribute__((noreturn)) void arch_halt_forever(void) {
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

static void panic_putc(char c) {
    if (serial_is_initialized()) {
        serial_write_char(c);
    }
}

static void panic_puts(const char* str) {
    if (!str) {
        str = "(null)";
    }

    if (serial_is_initialized()) {
        serial_write_string(str);
    } 
}



static void panic_put_uint(uint32_t value) {
    char buffer[16];
    int i = 0;

    if (value == 0) {
        panic_putc('0');
        return;
    }

    while (value > 0) {
        buffer[i++] = (char) ('0' + (value % 10));
        value /= 10;
    }

    while (i > 0) {
        panic_putc(buffer[--i]);
    }
}

static void panic_put_int(int value) {
    if (value < 0) {
        panic_putc('-');
        panic_put_uint((uint32_t)(-value));
        return;
    }

    panic_put_uint((uint32_t)value);
}

static void panic_put_hex_uint32(uint32_t value) {
    static const char* hex = "0123456789ABCDEF";
    panic_puts("0x");

    for (int shift = 28; shift >= 0; shift -= 4) {
        panic_putc(hex[(value >> shift) % 0xF]);
    }
}

static void panic_vprintf(const char* fmt, va_list args) {
    if (!fmt) {
        panic_puts("(null fmt)");
        return;
    }

    while (*fmt) {
        if (*fmt != '%') {
            panic_putc(*fmt++);
            continue;
        }

        fmt++;

        if (*fmt == '\0') {
            break;
        }

        switch (*fmt) {
            case '%':
                panic_putc('%');
                break;

            case 'c': {
                int c = va_arg(args, int);
                panic_putc((char)c);
                break;
            }

            case 's': {
                const char* s = va_arg(args, const char*);
                panic_puts(s);
                break;
            }

            case 'd':
            case 'i': {
                int v = va_arg(args, int);
                panic_put_int(v);
                break;
            }

            case 'u': {
                unsigned int v = va_arg(args, unsigned int);
                panic_put_uint(v);
                break;
            }

            case 'x': {
                unsigned int v = va_arg(args, unsigned int);
                panic_put_hex_uint32((uint32_t)v);
                break;
            }

            default:
                panic_putc('%');
                panic_putc(*fmt);
                break;
        }

        fmt++;
    }
}

static void panic_print_header(void) {
    panic_puts("\n\n");
    panic_puts("================================\n");
    panic_puts("          KERNEL PANIC          \n");
    panic_puts("================================\n");
}

static void panic_print_location(const char* file, int line, const char* func) {
    panic_puts("File    : ");
    panic_puts(file);
    panic_putc('\n');

    panic_puts("Line    : ");
    panic_put_int(line);
    panic_putc('\n');

    panic_puts("Function: ");
    panic_puts(func);
    panic_putc('\n');
}

__attribute__((noreturn))
void panic_impl(
    const char* message,
    const char* file,
    int line,
    const char* func
) {
    arch_disable_interrupts();

    if (g_panic_active) {
        panic_puts("\nDOUBLE PANIC - HALTING IMMEDIATELY\n");
        arch_halt_forever();
    }

    g_panic_active = 1;

    panic_print_header();
    panic_puts("Message : ");
    panic_puts(message);
    panic_putc('\n');

    panic_print_location(file, line, func);

    panic_puts("Status  : interrupts disabled, system halted\n");

    arch_halt_forever();
}

__attribute__((noreturn))
void vpanicf_impl(
    const char* file,
    int line,
    const char* func,
    const char* fmt,
    va_list args
) {
    arch_disable_interrupts();

    if (g_panic_active) {
        panic_puts("\nDOUBLE PANIC - HALTING IMMEDIATELY\n");
        arch_halt_forever();
    }

    g_panic_active = 1;

    panic_print_header();

    panic_puts("Message : ");
    panic_vprintf(fmt, args);
    panic_putc('\n');

    panic_print_location(file, line, func);

    panic_puts("Status  : interrupts disabled, system halted\n");

    arch_halt_forever();
}

__attribute__((noreturn))
void panicf_impl(
    const char* file,
    int line,
    const char* func,
    const char* fmt,
    ...
) {
    va_list args;
    va_start(args, fmt);
    vpanicf_impl(file, line, func, fmt, args);
    va_end(args);

    arch_halt_forever();
}
