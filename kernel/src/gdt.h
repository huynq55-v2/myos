// gdt.h

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Standard GDT entry (8 bytes)
typedef struct
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity; // Granularity flags
    uint8_t base_high;
} __attribute__((packed)) GDTEntry;

typedef struct
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) GDTR;

extern void setGdt(uint16_t limit, uint64_t base);
extern void reloadSegments(void);

void init_gdt();

#endif // GDT_H
