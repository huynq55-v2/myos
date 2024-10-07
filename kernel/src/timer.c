// timer.c
#include "timer.h"
#include "apic.h" // Giả định bạn có module APIC để lập trình APIC timer
#include "klibc.h"
#include "graphics.h"

// Khởi tạo bộ định thời sử dụng APIC timer
void timer_init(uint32_t frequency) {
    // Tính toán giá trị reload dựa trên tần số APIC timer
    uint32_t apic_timer_frequency = apic_get_timer_frequency(); // Hàm này cần được triển khai
    uint64_t count = apic_timer_frequency / frequency;

    if (count > 0xFFFFFFFF) {
        kprintf("Timer: Count value too large\n");
        while (1) {}
    }

    // Thiết lập APIC timer
    apic_set_timer_mode(APIC_TIMER_MODE_PERIODIC);
    apic_set_timer_divide(APIC_TIMER_DIVIDE_1);
    apic_set_timer_initial_count((uint32_t)count);

    // Bật APIC timer
    apic_enable_timer();
}

// Hàm gửi End Of Interrupt (EOI)
void send_eoi() {
    apic_send_eoi(); // Hàm này cần được triển khai theo hệ thống của bạn
}
