#include "gdt.h"
#include "tss.h"
#include "klibc.h"

#define GDT_SIZE 5 // Null, code, data, TSS (occupies 2 entries)

struct gdt_entry gdt[GDT_SIZE];
struct gdt_ptr gdtp;

extern tss_t tss;

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_set_tss(int num, uint64_t base, uint32_t limit) {
    struct tss_entry* tss_desc = (struct tss_entry*)&gdt[num];
    tss_desc->limit_low = limit & 0xFFFF;
    tss_desc->base_low = base & 0xFFFF;
    tss_desc->base_middle = (base >> 16) & 0xFF;
    tss_desc->access = 0x89; // Present, DPL=0, Type=9 (Available 64-bit TSS)
    tss_desc->granularity = ((limit >> 16) & 0x0F);
    tss_desc->base_high = (base >> 24) & 0xFF;
    tss_desc->base_upper = (base >> 32) & 0xFFFFFFFF;
    tss_desc->reserved = 0;
}

void gdt_init() {
    gdtp.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdtp.base = (uint64_t)&gdt;

    memset(&gdt, 0, sizeof(struct gdt_entry) * 5);

    gdt_set_gate(0, 0, 0, 0, 0);                   // Null segment
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xA0);       // Code segment
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xA0);       // Data segment

    // TSS occupies entries 3 and 4
    uint64_t tss_base = (uint64_t)&tss;
    uint32_t tss_limit = sizeof(tss_t) - 1;

    gdt_set_tss(3, tss_base, tss_limit);

    // Load GDT into CPU
    gdt_flush((uint64_t)&gdtp);
}
