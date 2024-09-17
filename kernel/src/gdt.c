#include "gdt.h"
#include <stdint.h>

struct gdt_entry gdt[3];
struct gdt_ptr gdtp;

// Hàm thiết lập một mục trong GDT
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);             // Địa chỉ cơ sở thấp
    gdt[num].base_middle = (base >> 16) & 0xFF;      // Địa chỉ cơ sở trung bình
    gdt[num].base_high = (base >> 24) & 0xFF;        // Địa chỉ cơ sở cao

    gdt[num].limit_low = (limit & 0xFFFF);           // Giới hạn thấp
    gdt[num].granularity = (limit >> 16) & 0x0F;     // Giới hạn cao

    gdt[num].granularity |= (gran & 0xF0);           // Các cờ granularity
    gdt[num].access = access;                        // Các cờ quyền truy cập
}

void gdt_init() {
    gdtp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gdtp.base = (uint64_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                   // Null segment
    gdt_set_gate(1, 0, 0, 0x9A, 0xA0);             // Code segment với bit L = 1
    gdt_set_gate(2, 0, 0, 0x92, 0x80);             // Data segment

    gdt_flush((uint64_t)&gdtp);
}
