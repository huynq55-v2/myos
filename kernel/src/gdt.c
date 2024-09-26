#include "gdt.h"
#include "stdint.h"
#include "tss.h"

uint8_t gdt[sizeof(GDTEntry32) * GDT_ENTRIES + sizeof(GDTEntry64)];
GDTR gdt_ptr;

// Function to set a GDT entry
void set_gdt_entry(GDTEntry32 *entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity)
{
    // For code and data segments in 64-bit mode, base and limit are ignored
    entry->limit_low = 0;
    entry->base_low = 0;
    entry->base_middle = 0;
    entry->access = access;
    entry->granularity = granularity;
    entry->base_high = 0;
}

void set_tss_descriptor(GDTEntry64 *entry, uint64_t base, uint32_t limit)
{
    entry->limit_low = limit & 0xFFFF;
    entry->base_low = base & 0xFFFF;
    entry->base_middle1 = (base >> 16) & 0xFF;
    entry->access = 0x89; // Present, privilege level 0, type 9 (available 64-bit TSS)
    entry->granularity = ((limit >> 16) & 0x0F);
    entry->base_middle2 = (base >> 24) & 0xFF;
    entry->base_high = (base >> 32) & 0xFFFFFFFF;
    entry->reserved = 0;
}

void init_gdt() {
    // Pointers to the entries within the gdt array
    GDTEntry32 *entries32 = (GDTEntry32 *)gdt;
    GDTEntry64 *tss_entry = (GDTEntry64 *)(gdt + sizeof(GDTEntry32) * GDT_ENTRIES);

    // Set up code and data segments
    set_gdt_entry(&entries32[0], 0, 0, 0, 0);          // Null segment
    set_gdt_entry(&entries32[1], 0, 0, 0x9A, 0x20);    // Kernel code segment
    set_gdt_entry(&entries32[2], 0, 0, 0x92, 0x00);    // Kernel data segment
    set_gdt_entry(&entries32[3], 0, 0, 0xFA, 0x20);    // User code segment
    set_gdt_entry(&entries32[4], 0, 0, 0xF2, 0x00);    // User data segment

    // Initialize the TSS
    init_tss();
    set_tss_descriptor(tss_entry, (uint64_t)&tss, sizeof(TSS));

    // Set up the GDTR
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint64_t)&gdt;

    // Load the GDT
    setGdt(gdt_ptr.limit, gdt_ptr.base);

    // Reload the segment registers
    reloadSegments();

    // Load the TSS (selector = index * 8, TSS is at index 5)
    load_tss(0x28);
}