// tss.c

#include "tss.h"
#include "gdt.h"
#include "stacks.h"

TSS tss;

void init_tss() {
    // Zero out the TSS structure
    for(int i = 0; i < sizeof(TSS)/8; i++) {
        ((uint64_t*)&tss)[i] = 0;
    }

    // Set the stack pointer for Ring 0 (kernel)
    tss.rsp0 = kernel_stack_top;

    // Optionally set IST pointers
    // For example, IST1 can be used for double faults
    tss.ist1 = ist1_stack_top;

    // Set the I/O Map Base Address to the end of the TSS
    tss.iomap_base = sizeof(TSS);
}

extern void loadTss();

void load_tss(uint16_t tss_selector) {
    __asm__ volatile (
        "ltr %%ax"  // Load Task Register (LTR)
        :
        : "a" (tss_selector)  // Pass the selector via the 'ax' register
    );
}
