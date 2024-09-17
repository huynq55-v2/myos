// idt.h
#ifndef IDT_H
#define IDT_H

#include <stdint.h>

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

void idt_init();
void isr_handler_c(uint64_t vector_number, uint64_t *stack);
void set_idt_gate(int vector, uint64_t handler, uint16_t selector, uint8_t type_attr, uint8_t ist);

#endif // IDT_H
