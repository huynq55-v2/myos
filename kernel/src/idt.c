// idt.c
#include "idt.h"
#include "klibc.h"
#include "graphics.h"

#define IDT_SIZE 256

idt_entry_t idt[IDT_SIZE];

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

extern void isr0();
extern void isr3();
extern void isr8();

void idt_init() {
    idtr.limit = (sizeof(idt_entry_t) * IDT_SIZE) - 1;
    idtr.base = (uint64_t)&idt;

    memset(&idt, 0, sizeof(idt_entry_t) * IDT_SIZE);

    set_idt_gate(0, (uint64_t)isr0, 0x08, 0x8E, 0); // Divide by zero
    set_idt_gate(3, (uint64_t)isr3, 0x08, 0x8E, 0); // Breakpoint
    set_idt_gate(8, (uint64_t)isr8, 0x08, 0x8E, 1); // Double fault uses IST1

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
    } else if (vector_number == 8) {
        kprintf("Double fault exception!\n");
    } else {
        kprintf("Unhandled exception: Vector %d\n", (int)vector_number);
    }

    // Dừng chương trình hoặc thực hiện xử lý phù hợp
    while (1) { __asm__ __volatile__("hlt"); }
}
