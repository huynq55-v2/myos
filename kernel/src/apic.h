// apic.h
#ifndef APIC_H
#define APIC_H

#include <stdint.h>

// Các giá trị định nghĩa cho chế độ của APIC timer
#define APIC_TIMER_MODE_ONE_SHOT   0x00000000
#define APIC_TIMER_MODE_PERIODIC   0x00020000
#define APIC_TIMER_MODE_TSC_DEADLINE 0x00040000

// Các giá trị định nghĩa cho bộ chia tần số của APIC timer
#define APIC_TIMER_DIVIDE_1       0x0000000B
#define APIC_TIMER_DIVIDE_2       0x00000000
#define APIC_TIMER_DIVIDE_4       0x00000001
#define APIC_TIMER_DIVIDE_8       0x00000002
#define APIC_TIMER_DIVIDE_16      0x00000003
#define APIC_TIMER_DIVIDE_32      0x00000008
#define APIC_TIMER_DIVIDE_64      0x00000009
#define APIC_TIMER_DIVIDE_128     0x0000000A

// Hàm lấy tần số APIC timer (giả định bạn sẽ triển khai)
uint32_t apic_get_timer_frequency();

// Hàm để thiết lập chế độ của APIC timer
void apic_set_timer_mode(uint32_t mode);

// Hàm để thiết lập giá trị chia của APIC timer
void apic_set_timer_divide(uint32_t divide);

// Hàm thiết lập giá trị đếm ban đầu của APIC timer
void apic_set_timer_initial_count(uint32_t count);

// Hàm bật APIC timer
void apic_enable_timer();

// Hàm gửi EOI (End Of Interrupt) tới APIC
void apic_send_eoi();

#endif // APIC_H
