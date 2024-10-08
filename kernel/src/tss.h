// tss.h

#ifndef TSS_H
#define TSS_H

#include <stdint.h>

typedef struct {
    uint32_t reserved1;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t iomap_base;
} __attribute__((packed, aligned(8))) TSS;

extern TSS tss;

void init_tss();

void load_tss(uint16_t tss_selector);

#endif // TSS_H
