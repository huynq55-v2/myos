// tss.S
.global tss_flush
tss_flush:
    movw $0x18, %ax    // Selector for TSS in GDT (index 3, so 3*8 = 0x18)
    ltr %ax            // Load TSS
    ret
