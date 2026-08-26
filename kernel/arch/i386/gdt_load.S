.section .text

.global gdt_load
.type gdt_load, @function
gdt_load:
  movl 4(%esp), %eax
  lgdt (%eax)

  ljmp $0x08, $gdt_reload_segments

gdt_reload_segments:
  movw $0x10, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  movw %ax, %ss

  ret

.size gdt_load, . - gdt_load

.global tss_load
.type tss_load, @function
tss_load:
  movw 4(%esp), %ax
  ltr %ax
  ret

.size tss_load, . - tss_load