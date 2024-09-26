// tests/tests.h
#ifndef TESTS_H
#define TESTS_H

#include <stdbool.h>

// Hàm để in kết quả kiểm thử
void test_print_result(const char *test_name, bool result);

void run_all_tests();

#endif // TESTS_H
