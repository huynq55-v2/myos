// idt.c
#include "idt.h"
#include "klibc.h"
#include "graphics.h"

idt_entry_t idt[IDT_SIZE];

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

// Khởi tạo isr_table với các địa chỉ ISR từ isr.S
void *isr_table[] = {
    &isr0,
    &isr1,
    &isr2,
    &isr3,
    &isr4,
    &isr5,
    &isr6,
    &isr7,
    &isr8,
    &isr9,
    &isr10,
    &isr11,
    &isr12,
    &isr13,
    &isr14,
    &isr15,
    &isr16,
    &isr17,
    &isr18,
    &isr19,
    &isr20,
    &isr21,
    &isr22,
    &isr23,
    &isr24,
    &isr25,
    &isr26,
    &isr27,
    &isr28,
    &isr29,
    &isr30,
    &isr31
};

void idt_init() {
    idtr.limit = (sizeof(idt_entry_t) * IDT_SIZE) - 1;
    idtr.base = (uint64_t)&idt;

    memset(&idt, 0, sizeof(idt_entry_t) * IDT_SIZE);

    for (int i = 0; i < 32; i++) {
        set_idt_gate(i, (uint64_t)isr_table[i], 0x08, 0x8E, 0);
    }

    // Thiết lập IST cho double fault handler (vector 8)
    set_idt_gate(8, (uint64_t)isr_table[8], 0x08, 0x8E, 1);


    __asm__ __volatile__ ("lidt %0" : : "m"(idtr));
    __asm__ __volatile__ ("sti");
}

void isr_handler_c(uint64_t vector_number, isr_stack_t *stack) {
    kprintf("Exception: Vector %d\n", (int)vector_number);
    kprintf("RIP: %lx, CS: %lx, RFLAGS: %lx\n", stack->rip, stack->cs, stack->rflags);

    // If there's an error code, print it
    if (vector_number == 8 || (vector_number >= 10 && vector_number <= 14) || vector_number == 17 || vector_number == 30) {
        kprintf("Error Code: %lx\n", stack->error_code);
    }

    // Halt the system or perform appropriate handling
    while (1) { __asm__ __volatile__("hlt"); }
}
