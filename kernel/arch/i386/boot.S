# setting constants for multiboot headers
.set ALIGN, 	1<<0 # left shift zero bits ALIGN = 1
.set MEMINFO, 	1<<1 # left shift 1 bit MEMINFO = 2 
.set FLAGS,	ALIGN | MEMINFO # combining ALIGN and MEMINFO, FLAGS = 3
.set MAGIC,	0x1BADB002 # Magic address for multiboot v1
.set CHECKSUM,	-(MAGIC + FLAGS) # checksum == 0	

# section for multiboot
.section .multiboot
.align 		4
.long		MAGIC
.long		FLAGS
.long 		CHECKSUM

# reserve stack for initial thread
.section .bss
.align 		16
stack_bottom:
.skip 		16384 # 16 KiB
stack_top:

# kernel entry point
.section .text
.global _start
.type _start, @function
_start:
	movl $stack_top, %esp

	# call _init

	pushl %ebx
	pushl %eax
	
	call kernel_main

	cli
1:	hlt
	jmp 1b

.size _start, . - _start
