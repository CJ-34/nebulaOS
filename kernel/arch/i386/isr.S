.set KERNEL_DATA_SELECTOR, 0x10

.section .text

  .global isr_exception_0
  .type isr_exception_0, @function
  isr_exception_0:
      pushl $0
      pushl $0
      jmp isr_common

  .global isr_exception_6
  .type isr_exception_6, @function
  isr_exception_6:
      pushl $0
      pushl $6
      jmp isr_common

    .global isr_exception_13
    .type isr_exception_13, @function
    isr_exception_13:
        pushl $13
        jmp isr_common

    .global isr_exception_14
    .type isr_exception_14, @function
    isr_exception_14:
        pushl $14
        jmp isr_common

    .global irq_keyboard
    .type irq_keyboard, @function
    irq_keyboard:
        pusha
        cld

        call keyboard_handle_irq

        popa
        iret

    .size irq_keyboard, . - irq_keyboard

    .global irq_timer
    .type irq_timer, @function
    irq_timer:
        pusha
        cld

        call pit_handle_irq

        popa
        iret
    .size irq_timer, . - irq_timer

    .global syscall_entry
    .type syscall_entry, @function
    syscall_entry:
        push %ds
        push %es
        push %fs
        push %gs

        pusha
        cld

        movw $KERNEL_DATA_SELECTOR, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %fs
        movw %ax, %gs

        pushl %esp
        call syscall_handler
        addl $4, %esp

        popa

        pop %gs
        pop %fs
        pop %es
        pop %ds
        iret
    .size syscall_entry, . - syscall_entry

  .type isr_common, @function
  isr_common:
      pusha
      cld

      pushl %esp
      call exception_handler
      addl $4, %esp

      popa
      addl $8, %esp
      iret
