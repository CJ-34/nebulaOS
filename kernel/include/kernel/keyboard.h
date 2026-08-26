#ifndef _KERNEL_KEYBOARD_H
#define _KERNEL_KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

void keyboard_init(void);

void keyboard_handle_irq(void);

bool keyboard_pop_scancode(uint8_t *scancode);

bool keyboard_pop_char(char *character);

#endif