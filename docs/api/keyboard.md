# Keyboard API

Header: `kernel/include/kernel/keyboard.h`

`keyboard_init()` resets the PS/2 scan-code ring buffer and modifier state.
`keyboard_handle_irq()` must run for IRQ1: it reads port `0x60`, queues one
scan code, and acknowledges the PIC.

`keyboard_pop_scancode()` drains raw Set-1 scan codes. `keyboard_pop_char()`
translates basic make codes into ASCII and handles left/right Shift, Enter,
Backspace, and printable US-layout keys. Extended keys and locking modifiers
are not implemented.
