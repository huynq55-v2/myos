// tests/tests.c
#include "graphics.h"
#include <stdbool.h>
#include "gdt.h"
#include "tss.h"
#include "stacks.h"

extern GDTR gdt_ptr;  // Con trỏ đến GDTR hiện tại trong kernel
extern TSS tss;       // Task State Segment hiện tại

// Hàm chạy tất cả kiểm thử
void run_all_tests() {
    kprintf("=== Starting All Tests ===\n");

    test_gdt();
    test_tss();

    kprintf("=== All Tests Completed ===\n");
}

// Hàm để in kết quả kiểm thử
void test_print_result(const char *test_name, bool result) {
    if (result) {
        kprintf("PASS: %s\n", test_name);
    } else {
        kprintf("FAIL: %s\n", test_name);
    }
}

// Kiểm thử GDT: kiểm tra xem GDT đã được nạp đúng chưa
void test_gdt() {
    bool result = true;

    // Kiểm tra kích thước của GDT
    if (gdt_ptr.limit != sizeof(GDTEntry32) * 5 + sizeof(GDTEntry64) - 1) {
        result = false;
    }

    // Bạn có thể kiểm tra thêm các mục trong GDT ở đây, ví dụ như access flags của segment
    // if (entries32[1].access != 0x9A) { // Kernel code segment
    //     result = false;
    // }

    test_print_result("GDT Initialization Test", result);
}

// Kiểm thử TSS: kiểm tra xem TSS đã được nạp đúng chưa
void test_tss() {
    bool result = true;

    // Kiểm tra xem stack pointer cho kernel có được khởi tạo đúng không
    if (tss.rsp0 != kernel_stack_top) {
        result = false;
    }

    // Kiểm tra xem IST1 có được khởi tạo đúng không
    if (tss.ist1 != ist1_stack_top) {
        result = false;
    }

    // Kiểm tra I/O map base
    if (tss.iomap_base != sizeof(TSS)) {
        result = false;
    }

    test_print_result("TSS Initialization Test", result);
}