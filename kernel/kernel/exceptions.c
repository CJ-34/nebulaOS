#include <kernel/panic.h>
#include <kernel/exceptions.h>

static uint32_t read_cr2(void) {
    uint32_t value;

    __asm__ volatile (
        "movl %%cr2, %0"
        : "=r"(value)
    );

    return value;
}

static const char* exception_name(uint32_t vector) {
    switch (vector) {
        case 0:
            return "Divide error";
        case 6:
            return "Invalid opcode";
        case 13:
            return "General protection fault";
        case 14:
            return "Page fault";
        default:
            return "Unknown exception";
    }
}

__attribute__((noreturn))
void exception_handler(struct exception_frame* frame) {
    if (frame->vector == 14) {
        PANICF("Page fault at %x (error code %x)", read_cr2(), frame->error_code);
    }

    PANICF("CPU exception %d: %s (error code %x)",
        frame->vector,
        exception_name(frame->vector),
        frame->error_code
    );
}