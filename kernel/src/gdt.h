// gdt.h

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Standard GDT entry (8 bytes)
// Updated GDT entry for 64-bit mode
typedef struct
{
    uint16_t limit_low;       // Bits 0-15 of the limit
    uint16_t base_low;        // Bits 0-15 of the base
    uint8_t base_middle;      // Bits 16-23 of the base
    uint8_t access;           // Access flags
    uint8_t granularity;      // Granularity and bits 16-19 of the limit
    uint8_t base_high;        // Bits 24-31 of the base
} __attribute__((packed)) GDTEntry32;

// For TSS and LDT descriptors, which are 16 bytes
typedef struct
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle1;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_middle2;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__((packed)) GDTEntry64;

typedef struct
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) GDTR;

extern void setGdt(uint16_t limit, uint64_t base);
extern void reloadSegments(void);

void init_gdt();

extern GDTEntry32 gdt_entries[];

#define GDT_ENTRIES 6 // Increased to 6 to include TSS

#endif // GDT_H
