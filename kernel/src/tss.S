// tss.S
.global tss_flush
tss_flush:
    movw $0x28, %ax    // Selector for TSS in GDT (index 5, so 5*8 = 0x28)
    ltr %ax            // Load TSS
    ret
