.section .text

.set USER_CODE_SELECTOR, 0x1B
.set USER_DATA_SELECTOR, 0x23

.global user_mode_enter
.type user_mode_enter, @function
user_mode_enter:
    movl 4(%esp), %edx
    movl 8(%esp), %ecx

    movw $USER_DATA_SELECTOR, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    pushl $USER_DATA_SELECTOR
    pushl %ecx

    pushfl
    popl %eax
    orl $0x200, %eax
    pushl %eax

    pushl $USER_CODE_SELECTOR
    pushl %edx
    iret

.size user_mode_enter, . - user_mode_enter
