#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Cấu trúc cho một GDT entry
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

// Khai báo extern để tránh multiple definition
extern struct gdt_entry gdt[3];
extern struct gdt_ptr gdtp;

extern void gdt_flush(uint64_t);

void gdt_init();

#endif
