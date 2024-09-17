// idt.c
#include "idt.h"
#include "klibc.h"
#include "graphics.h"

#define IDT_SIZE 256

idt_entry_t idt[IDT_SIZE];

// Cấu trúc của IDTR
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;

idtr_t idtr;

void set_idt_gate(int vector, uint64_t handler, uint16_t selector, uint8_t type_attr, uint8_t ist) {
    idt[vector].offset_low = handler & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].ist = ist & 0x7;
    idt[vector].type_attr = type_attr;
    idt[vector].offset_mid = (handler >> 16) & 0xFFFF;
    idt[vector].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0;
}

// Khai báo ngoại các hàm xử lý (được viết trong assembly)
extern void isr0();  // Hàm xử lý chia cho 0
extern void isr3();  // Hàm xử lý breakpoint

void idt_init() {
    // Thiết lập IDTR
    idtr.limit = (sizeof(idt_entry_t) * IDT_SIZE) - 1;
    idtr.base = (uint64_t)&idt;

    // Xóa toàn bộ IDT
    memset(&idt, 0, sizeof(idt_entry_t) * IDT_SIZE);

    // Thiết lập các hàm xử lý ngoại lệ
    set_idt_gate(0, (uint64_t)isr0, 0x08, 0x8E, 0); // Chia cho 0
    set_idt_gate(3, (uint64_t)isr3, 0x08, 0x8E, 0); // Breakpoint

    // Nạp IDT vào CPU
    __asm__ __volatile__ ("lidt %0" : : "m"(idtr));
}

void isr_handler_c(uint64_t vector_number, uint64_t *stack) {
    // Mã lỗi nằm ở vị trí stack[0]
    uint64_t error_code = stack[0];

    kprintf("An exception occurred!\n");
    kprintf("Vector: %d, Error Code: %d\n", (int)vector_number, (int)error_code);

    if (vector_number == 0) {
        kprintf("Division by zero exception!\n");
    } else if (vector_number == 3) {
        kprintf("Breakpoint exception!\n");
    } else {
        kprintf("Unhandled exception: Vector %d\n", (int)vector_number);
    }

    // Dừng chương trình hoặc thực hiện xử lý phù hợp
    while (1) { __asm__ __volatile__("hlt"); }
}
