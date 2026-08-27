# PIT API

Header: `kernel/include/kernel/pit.h`

`pit_init(frequency_hz)` programs PIT channel 0 using the 1,193,182 Hz input
clock. `pit_handle_irq()` increments the volatile tick counter and sends the
IRQ0 EOI. `pit_ticks()` returns that counter.

The counter may advance between separate reads; callers that compare times
should use unsigned subtraction.
