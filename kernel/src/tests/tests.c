// tests/tests.c
#include "graphics.h"
#include <stdbool.h>

// Khai báo các hàm kiểm thử
void test_gdt();
void test_tss();

void run_all_tests() {
    kprintf("=== Starting All Tests ===\n");
    kprintf("=== All Tests Completed ===\n");
}

void test_print_result(const char *test_name, bool result) {
    if (result) {
        kprintf("PASS: %s\n", test_name);
    } else {
        kprintf("FAIL: %s\n", test_name);
    }
}