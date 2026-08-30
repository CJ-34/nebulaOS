.section .text

.global task_switch
.type task_switch, @function
task_switch:
    movl 4(%esp), %eax
    movl 8(%esp), %edx

    pushfl
    push %ds
    push %es
    push %fs
    push %gs

    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

    movl %esp, (%eax)
    movl %edx, %esp

    popl %edi
    popl %esi
    popl %ebx
    popl %ebp

    pop %gs
    pop %fs
    pop %es
    pop %ds
    popfl

    ret

.size task_switch, . - task_switch