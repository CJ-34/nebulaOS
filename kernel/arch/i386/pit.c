#include <stdint.h>

#include <kernel/pic.h>
#include <kernel/pit.h>

#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIT_INPUT_HZ 1193182

static volatile uint32_t ticks;

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void pit_init(uint32_t frequency_hz) {
    uint16_t divisor = (uint16_t)(PIT_INPUT_HZ / frequency_hz);

    outb(PIT_COMMAND_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t pit_ticks(void) {
    return ticks;
}

void pit_handle_irq(void) {
    ticks++;
    pic_send_eoi(0);
}