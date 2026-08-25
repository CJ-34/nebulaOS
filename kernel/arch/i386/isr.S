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

    .global irq_timer
    .type irq_timer, @function
    irq_timer:
        pusha
        cld

        call pit_handle_irq

        popa
        iret
    .size irq_timer, . - irq_timer

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
