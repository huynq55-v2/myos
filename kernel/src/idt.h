// idt.h
#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef long ssize_t;

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

typedef struct {
    uint64_t error_code; // Only present if an error code is pushed
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
} isr_stack_t;

void idt_init();
void isr_handler_c(uint64_t vector_number, isr_stack_t *stack);
void set_idt_gate(int vector, uint64_t handler, uint16_t selector, uint8_t type_attr, uint8_t ist);

ssize_t syscall_handler_c(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3);

#define SYSCALL_VECTOR 0x80

extern void *isr_table[];

extern idt_entry_t idt[]; // Mảng IDT

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;

extern idtr_t idtr;

#define IDT_SIZE 256

#endif // IDT_H
