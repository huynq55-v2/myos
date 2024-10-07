// apic.c
#include "apic.h"
#include "klibc.h"  // Để sử dụng kprintf

// Địa chỉ của Local APIC, bạn có thể cần thay đổi địa chỉ này tùy thuộc vào hệ thống của bạn
#define APIC_BASE 0xFEE00000
#define APIC_LVT_TIMER (APIC_BASE + 0x320)
#define APIC_INITIAL_COUNT (APIC_BASE + 0x380)
#define APIC_CURRENT_COUNT (APIC_BASE + 0x390)
#define APIC_DIVIDE_CONFIG (APIC_BASE + 0x3E0)
#define APIC_EOI (APIC_BASE + 0xB0)

// Hàm lấy tần số APIC timer (giả định là giá trị mặc định hoặc tính toán theo hệ thống)
uint32_t apic_get_timer_frequency() {
    // Triển khai thuật toán lấy tần số của APIC timer
    return 1000000; // Ví dụ: 1 MHz
}

// Hàm thiết lập chế độ của APIC timer
void apic_set_timer_mode(uint32_t mode) {
    volatile uint32_t *apic_lvt_timer = (uint32_t *)APIC_LVT_TIMER;
    *apic_lvt_timer = (*apic_lvt_timer & ~0x00060000) | mode;
}

// Hàm thiết lập giá trị chia của APIC timer
void apic_set_timer_divide(uint32_t divide) {
    volatile uint32_t *apic_divide_config = (uint32_t *)APIC_DIVIDE_CONFIG;
    *apic_divide_config = divide;
}

// Hàm thiết lập giá trị đếm ban đầu của APIC timer
void apic_set_timer_initial_count(uint32_t count) {
    volatile uint32_t *apic_initial_count = (uint32_t *)APIC_INITIAL_COUNT;
    *apic_initial_count = count;
}

// Hàm bật APIC timer
void apic_enable_timer() {
    volatile uint32_t *apic_lvt_timer = (uint32_t *)APIC_LVT_TIMER;
    *apic_lvt_timer |= 0x10000;  // Bật APIC timer
}

// Hàm gửi EOI (End Of Interrupt) tới APIC
void apic_send_eoi() {
    volatile uint32_t *apic_eoi = (uint32_t *)APIC_EOI;
    *apic_eoi = 0;
}
