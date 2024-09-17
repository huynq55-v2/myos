// tests/test_idt.c

#include "tests.h"        // Bao gồm tệp header kiểm thử
#include "idt.h"
#include "graphics.h"
#include "klibc.h"

// Hàm giả lập xử lý ngắt
extern void isr0_handler();
extern void isr3_handler();
extern void isr8_handler();

// Biến để theo dõi các ngắt đã được xử lý
volatile int interrupt_handled[256] = {0};

// Các ISR giả lập để đánh dấu ngắt đã được xử lý
void isr0_handler() {
    interrupt_handled[0] = 1;
}

void isr3_handler() {
    interrupt_handled[3] = 1;
}

void isr8_handler() {
    interrupt_handled[8] = 1;
}

// Hàm để kích hoạt ngắt với vector 0
void trigger_interrupt0() {
    __asm__ __volatile__ (
        "int $0"           // Thực hiện ngắt với vector 0
        :
        :
        : "memory"
    );
}

// Hàm để kích hoạt ngắt với vector 3
void trigger_interrupt3() {
    __asm__ __volatile__ (
        "int $3"           // Thực hiện ngắt với vector 3
        :
        :
        : "memory"
    );
}

// Hàm để kích hoạt ngắt với vector 8
void trigger_interrupt8() {
    __asm__ __volatile__ (
        "int $8"           // Thực hiện ngắt với vector 8
        :
        :
        : "memory"
    );
}

void test_idt() {
    kprintf("Running IDT tests...\n");
    
    // Thiết lập các ISR giả lập
    set_idt_gate(0, (uint64_t)isr0_handler, 0x08, 0x8E, 0);
    set_idt_gate(3, (uint64_t)isr3_handler, 0x08, 0x8E, 0);
    set_idt_gate(8, (uint64_t)isr8_handler, 0x08, 0x8E, 1);

    // Kích hoạt các ngắt
    trigger_interrupt0(); // Divide by zero
    trigger_interrupt3(); // Breakpoint
    trigger_interrupt8(); // Double fault

    // Kiểm tra các ngắt đã được xử lý
    bool result0 = interrupt_handled[0];
    bool result3 = interrupt_handled[3];
    bool result8 = interrupt_handled[8];

    test_print_result("IDT - Divide by zero", result0);
    test_print_result("IDT - Breakpoint", result3);
    test_print_result("IDT - Double fault", result8);

    kprintf("IDT tests completed.\n");
}
