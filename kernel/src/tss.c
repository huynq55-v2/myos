// tss.c
#include "tss.h"
#include "klibc.h"

extern void tss_flush();

tss_t tss __attribute__((aligned(16)));

uint8_t ist1_stack[4096] __attribute__((aligned(16)));
uint8_t kernel_stack[4096] __attribute__((aligned(16)));

void tss_init() {
    memset(&tss, 0, sizeof(tss_t));

    tss.rsp0 = (uint64_t)&kernel_stack[4096];
    tss.ist1 = (uint64_t)&ist1_stack[4096];

    tss.iomap_base = sizeof(tss_t);

    tss_flush();
}
