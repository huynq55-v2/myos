// gdt.c

#include "gdt.h"
#include "stdint.h"
#include "tss.h"

// Định nghĩa biến toàn cục GDT và GDTR
GDT gdt;
GDTR gdt_ptr;

// Hàm thiết lập một mục GDT
void set_gdt_entry(GDTEntry32 *entry, uint8_t access, uint8_t granularity)
{
    entry->limit_low = 0;
    entry->base_low = 0;
    entry->base_middle = 0;
    entry->access = access;
    entry->granularity = granularity;
    entry->base_high = 0;
}

// Hàm thiết lập TSS Descriptor
void set_tss_descriptor(GDTEntry64 *entry, uint64_t base, uint32_t limit)
{
    entry->limit_low = limit & 0xFFFF;
    entry->base_low = base & 0xFFFF;
    entry->base_middle1 = (base >> 16) & 0xFF;
    entry->access = 0x89; // Present, DPL=0, Type=9 (Available 64-bit TSS)
    entry->granularity = ((limit >> 16) & 0x0F);
    entry->base_middle2 = (base >> 24) & 0xFF;
    entry->base_high = (base >> 32);
    entry->reserved = 0;
}

void init_gdt() {
    // Thiết lập các segment code và data
    set_gdt_entry(&gdt.entries32[0], 0, 0);          // Null segment
    set_gdt_entry(&gdt.entries32[1], 0x9A, 0x20);    // Kernel code segment
    set_gdt_entry(&gdt.entries32[2], 0x92, 0x00);    // Kernel data segment
    set_gdt_entry(&gdt.entries32[3], 0xFA, 0x20);    // User code segment
    set_gdt_entry(&gdt.entries32[4], 0xF2, 0x00);    // User data segment

    // Khởi tạo TSS
    init_tss();
    set_tss_descriptor(&gdt.tss_entry, (uint64_t)&tss, sizeof(TSS));

    // Thiết lập GDTR
    gdt_ptr.limit = sizeof(GDT) - 1;
    gdt_ptr.base = (uint64_t)&gdt;

    // Load GDT
    setGdt(gdt_ptr.limit, gdt_ptr.base);

    // Reload segment registers
    reloadSegments();

    // Load TSS (selector = index * 8, TSS ở index 5)
    load_tss(0x28);
}
