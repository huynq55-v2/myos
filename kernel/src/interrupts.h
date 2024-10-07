// interrupts.h
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
#include "idt.h"

// Định nghĩa vector ngắt timer (ví dụ: 32)
#define TIMER_INTERRUPT_VECTOR 32

// Hàm đăng ký trình xử lý ngắt
void register_interrupt_handler(uint8_t vector, void (*handler)(uint64_t, isr_stack_t *));

#endif // INTERRUPTS_H
