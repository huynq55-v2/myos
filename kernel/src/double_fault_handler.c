// double_fault_handler.c
#include "idt.h"
#include "klibc.h"
#include "graphics.h"

// Hàm xử lý Double Fault
void double_fault_handler_c(uint64_t vector_number, isr_stack_t *stack) {
    kprintf("Double Fault detected!\n");
    kprintf("RIP: %lx, CS: %lx, RFLAGS: %lx\n", stack->rip, stack->cs, stack->rflags);
    // Thêm xử lý cụ thể nếu cần thiết
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
