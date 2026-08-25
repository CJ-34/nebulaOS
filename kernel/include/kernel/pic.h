#ifndef _KERNEL_PIC_H
#define _KERNEL_PIC_H

#include <stdint.h>

#define PIC_IRQ_BASE 32

void pic_init(void);
void pic_unmask_irq(uint8_t irq);
void pic_send_eoi(uint8_t irq);

#endif