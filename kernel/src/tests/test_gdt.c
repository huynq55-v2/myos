// tests/test_gdt.c
#include "gdt.h"
#include "graphics.h"
#include "klibc.h"
#include "tests.h"

extern struct gdt_entry gdt[];
extern struct gdt_ptr gdtp;

void test_gdt() {
    kprintf("Running GDT tests...\n");

    // Kiểm tra các mục GDT đã được thiết lập đúng
    bool null_segment = (gdt[0].limit_low == 0) && (gdt[0].base_low == 0) &&
                        (gdt[0].base_middle == 0) && (gdt[0].access == 0) &&
                        (gdt[0].granularity == 0) && (gdt[0].base_high == 0);

    bool code_segment = ((gdt[1].access & 0xFE) == 0x9A) && (gdt[1].granularity == 0xAF);
    bool data_segment = ((gdt[2].access & 0xFE) == 0x92) && (gdt[2].granularity == 0xAF);

    test_print_result("GDT - Null Segment", null_segment);
    test_print_result("GDT - Code Segment", code_segment);
    test_print_result("GDT - Data Segment", data_segment);

    kprintf("GDT tests completed.\n");
}
