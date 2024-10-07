// idt.c
#include "idt.h"
#include "klibc.h"
#include "graphics.h"
#include "syscall_handler.h"

// Định nghĩa các trình xử lý ngoại lệ (vector 0-31)
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

// Trình xử lý syscall
extern void syscall_handler();

// Khai báo bảng trình xử lý ngắt
void (*interrupt_handlers[IDT_SIZE])(uint64_t, isr_stack_t *) = {0};

// Mảng ISR từ isr.S
void *isr_table[] = {
    &isr0, &isr1, &isr2, &isr3, &isr4, &isr5, &isr6, &isr7,
    &isr8, &isr9, &isr10, &isr11, &isr12, &isr13, &isr14, &isr15,
    &isr16, &isr17, &isr18, &isr19, &isr20, &isr21, &isr22, &isr23,
    &isr24, &isr25, &isr26, &isr27, &isr28, &isr29, &isr30, &isr31
};

// Hàm thiết lập một gate trong IDT
void set_idt_gate(int vector, uint64_t handler, uint16_t selector, uint8_t type_attr, uint8_t ist) {
    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].ist = ist & 0x7;
    idt[vector].type_attr = type_attr;
    idt[vector].offset_mid = (handler >> 16) & 0xFFFF;
    idt[vector].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0;
}

// Hàm đăng ký trình xử lý ngắt
void register_interrupt_handler(uint8_t vector, void (*handler)(uint64_t, isr_stack_t *)) {
    if (vector >= IDT_SIZE) {
        kprintf("IDT: Invalid interrupt vector %d\n", vector);
        return;
    }
    interrupt_handlers[vector] = handler;
    set_idt_gate(vector, (uint64_t)isr_table[vector], 0x08, 0x8E, 0);
}

idt_entry_t idt[IDT_SIZE]; // Định nghĩa mảng IDT
idtr_t idtr; // Định nghĩa IDTR

// Hàm khởi tạo IDT
void idt_init() {
    idtr.limit = (sizeof(idt_entry_t) * IDT_SIZE) - 1;
    idtr.base = (uint64_t)&idt;

    memset(&idt, 0, sizeof(idt_entry_t) * IDT_SIZE);

    // Thiết lập trình xử lý ngoại lệ (vector 0-31)
    for (int i = 0; i < 32; i++) {
        set_idt_gate(i, (uint64_t)isr_table[i], 0x08, 0x8E, 0);
        register_interrupt_handler(i, isr_handler_c); // Handler chung cho ngoại lệ
    }

    // Thiết lập IST cho double fault handler (vector 8)
    set_idt_gate(8, (uint64_t)isr_table[8], 0x08, 0x8E, 1);
    register_interrupt_handler(8, isr_handler_c); // Handler chung cho double fault

    // Thiết lập trình xử lý syscall (vector SYSCALL_VECTOR)
    set_idt_gate(SYSCALL_VECTOR, (uint64_t)syscall_handler, 0x08, 0xEE, 0);
    register_interrupt_handler(SYSCALL_VECTOR, syscall_handler_c); // Handler syscall

    // Thiết lập trình xử lý ngắt timer (vector TIMER_INTERRUPT_VECTOR)
    extern void timer_interrupt_handler_c(uint64_t, isr_stack_t *);
    register_interrupt_handler(TIMER_INTERRUPT_VECTOR, timer_interrupt_handler_c);

    // Load IDT
    __asm__ __volatile__ ("lidt %0" : : "m"(idtr));
    __asm__ __volatile__ ("sti");
}

// Hàm xử lý ngắt trong C
void isr_handler_c(uint64_t vector_number, isr_stack_t *stack) {
    if (interrupt_handlers[vector_number]) {
        interrupt_handlers[vector_number](vector_number, stack);
    } else {
        // Nếu không có trình xử lý cụ thể, xử lý ngoại lệ chung
        kprintf("Unhandled Interrupt: Vector %d\n", (int)vector_number);
        kprintf("RIP: %lx, CS: %lx, RFLAGS: %lx\n", stack->rip, stack->cs, stack->rflags);

        // Nếu có mã lỗi, in ra
        if (vector_number == 8 || (vector_number >= 10 && vector_number <= 14) || vector_number == 17 || vector_number == 30) {
            kprintf("Error Code: %lx\n", stack->error_code);
        }

        // Dừng hệ thống
        while (1) { __asm__ __volatile__("hlt"); }
    }
}
