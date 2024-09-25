// tests/tests.c
#include "graphics.h"

// Khai báo các hàm kiểm thử
void run_klibc_tests();
void test_idt();
void test_gdt();

void run_all_tests() {
    kprintf("=== Starting All Tests ===\n");
    run_klibc_tests();
    test_idt();
    kprintf("=== All Tests Completed ===\n");
}
