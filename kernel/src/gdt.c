#include "gdt.h"
#include "stdint.h"

#define GDT_ENTRIES 5

GDTEntry gdt_entries[GDT_ENTRIES];
GDTR gdt_ptr;

// Function to set a GDT entry
void set_gdt_entry(GDTEntry *entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity)
{
    entry->limit_low = limit & 0xFFFF;
    entry->base_low = base & 0xFFFF;
    entry->base_middle = (base >> 16) & 0xFF;
    entry->access = access;
    entry->granularity = (limit >> 16) & 0x0F;
    entry->granularity |= granularity;
    entry->base_high = (base >> 24) & 0xFF;
}

void init_gdt() {
    gdt_ptr.limit = sizeof(GDTEntry) * GDT_ENTRIES - 1;
    gdt_ptr.base  = (uint64_t)gdt_entries;

    set_gdt_entry(&gdt_entries[0], 0, 0, 0, 0);                    // Null segment
    set_gdt_entry(&gdt_entries[1], 0, 0xFFFFFFFF, 0x9A, 0xAF);     // Kernel Code segment (PL0)
    set_gdt_entry(&gdt_entries[2], 0, 0xFFFFFFFF, 0x92, 0xCF);     // Kernel Data segment (PL0)
    set_gdt_entry(&gdt_entries[3], 0, 0xFFFFFFFF, 0xFA, 0xAF);     // User Code segment (PL3)
    set_gdt_entry(&gdt_entries[4], 0, 0xFFFFFFFF, 0xF2, 0xCF);     // User Data segment (PL3)

    setGdt(gdt_ptr.limit, gdt_ptr.base);
    reloadSegments();
}