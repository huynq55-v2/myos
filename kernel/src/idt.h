// idt.h
#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef long ssize_t;

// Kích thước IDT
#define IDT_SIZE 256

// Vector cho syscall
#define SYSCALL_VECTOR 0x80

// Vector cho timer interrupt
#define TIMER_INTERRUPT_VECTOR 32

// Cấu trúc của một mục IDT (Interrupt Gate)
typedef struct {
    uint16_t offset_low;    // 16 bit thấp của địa chỉ hàm xử lý
    uint16_t selector;      // Selector của đoạn mã trong GDT
    uint8_t ist;            // Chỉ số Interrupt Stack Table (0 nếu không sử dụng)
    uint8_t type_attr;      // Loại và thuộc tính
    uint16_t offset_mid;    // 16 bit giữa của địa chỉ hàm xử lý
    uint32_t offset_high;   // 32 bit cao của địa chỉ hàm xử lý
    uint32_t zero;          // Dự trữ
} __attribute__((packed)) idt_entry_t;

// Cấu trúc IDTR
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;

// Cấu trúc stack khi xảy ra ISR (Interrupt Service Routine)
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t rflags, cs, rip;
    uint64_t error_code; // Chỉ có nếu ISR có mã lỗi
} isr_stack_t;

// Hàm thiết lập một gate trong IDT
void set_idt_gate(int vector, uint64_t handler, uint16_t selector, uint8_t type_attr, uint8_t ist);

// Hàm đăng ký trình xử lý ngắt
void register_interrupt_handler(uint8_t vector, void (*handler)(uint64_t, isr_stack_t *));

// Hàm khởi tạo IDT
void idt_init();

// Hàm xử lý ngắt trong C
void isr_handler_c(uint64_t vector_number, isr_stack_t *stack);

void double_fault_handler_c(uint64_t vector_number, isr_stack_t *stack);

// Hàm xử lý syscall trong C
ssize_t syscall_handler_c(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3);

// Khai báo các trình xử lý ISR từ isr.S
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

// Khai báo IDT và IDTR
extern idt_entry_t idt[];
extern idtr_t idtr;

// Khai báo bảng ISR
extern void *isr_table[];

#endif // IDT_H
