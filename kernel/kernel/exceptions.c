#include <kernel/panic.h>
#include <kernel/exceptions.h>

static const char* exception_name(uint32_t vector) {
    switch (vector) {
        case 0:
            return "Divide error";
        case 6:
            return "Invalid opcode";
        default:
            return "Unknown exception";
    }
}

__attribute__((noreturn))
void exception_handler(struct exception_frame* frame) {
    PANICF("CPU exception %d: %s",
        frame->vector,
        exception_name(frame->vector)
    );
}