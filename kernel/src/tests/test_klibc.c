// tests/test_klibc.c
#include "klibc.h"
#include "graphics.h"
#include "tests.h"

// Hàm để in kết quả kiểm thử
void test_print_result(const char *test_name, bool result) {
    if (result) {
        kprintf("PASS: %s\n", test_name);
    } else {
        kprintf("FAIL: %s\n", test_name);
    }
}

void test_memcpy() {
    char src[] = "Hello, memcpy!";
    char dest[20];
    memcpy(dest, src, sizeof(src));
    bool result = (memcmp(dest, src, sizeof(src)) == 0);
    test_print_result("memcpy", result);
}

void test_memset() {
    char buffer[10];
    memset(buffer, 'A', sizeof(buffer));
    bool result = true;
    for (int i = 0; i < 10; i++) {
        if (buffer[i] != 'A') {
            result = false;
            break;
        }
    }
    test_print_result("memset", result);
}

void test_memmove() {
    char buffer[20] = "abcdefghijklmnopqrstuvwxyz";
    memmove(buffer + 5, buffer, 10);
    bool result = (memcmp(buffer + 5, "abcdefghij", 10) == 0);
    test_print_result("memmove", result);
}

void test_memcmp() {
    char a[] = "test";
    char b[] = "test";
    char c[] = "test1";
    bool result1 = (memcmp(a, b, 4) == 0);
    bool result2 = (memcmp(a, c, 5) < 0);
    test_print_result("memcmp equal", result1);
    test_print_result("memcmp less than", result2);
}

void run_klibc_tests() {
    kprintf("Running klibc tests...\n");
    test_memcpy();
    test_memset();
    test_memmove();
    test_memcmp();
    kprintf("klibc tests completed.\n");
}
