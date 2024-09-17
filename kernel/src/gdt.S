.global gdt_flush
gdt_flush:
    movq %rdi, %rax
    lgdt (%rax)

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    pushq $0x08
    pushq $.flush
    retfq
.flush:
    ret
