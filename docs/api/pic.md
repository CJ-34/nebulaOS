# PIC API

Header: `kernel/include/kernel/pic.h`

`pic_init()` remaps the legacy 8259 PIC to vectors beginning at
`PIC_IRQ_BASE` (`32`) and masks all IRQs. `pic_unmask_irq(irq)` enables one
IRQ line. `pic_send_eoi(irq)` acknowledges a serviced IRQ, including the slave
PIC when required.

NebulaOS currently unmasks only IRQ0 (PIT) and IRQ1 (keyboard).
