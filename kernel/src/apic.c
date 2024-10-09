// apic.c
#include "apic.h"

// Hàm để ghi vào một thanh ghi APIC (sử dụng địa chỉ ảo)
static inline void apic_write(uint32_t address_phys, uint32_t value) {
    volatile uint32_t *address_virt = (volatile uint32_t *)PHYS_TO_VIRT(address_phys);
    *address_virt = value;
}

// Hàm để đọc từ một thanh ghi APIC (sử dụng địa chỉ ảo)
static inline uint32_t apic_read(uint32_t address_phys) {
    volatile uint32_t *address_virt = (volatile uint32_t *)PHYS_TO_VIRT(address_phys);
    return *address_virt;
}

// Hàm gửi tín hiệu EOI (End Of Interrupt)
void apic_eoi() {
    apic_write(APIC_REG_EOI_PHYS, 0);
}

// ISR cho bộ hẹn giờ APIC
void apic_timer_isr() {
    // Xử lý ngắt bộ hẹn giờ ở đây
    // Ví dụ: Cập nhật đồng hồ hệ thống, chuyển đổi tiến trình, v.v.
    
    // Gửi EOI để thông báo rằng ngắt đã được xử lý
    apic_eoi();
}

void delay_ms(uint32_t ms) {
    // Một vòng lặp trống dùng để delay, không chính xác nhưng đơn giản
    for (uint32_t i = 0; i < ms * 1000; i++) {
        asm volatile("nop"); // Nop để tránh tối ưu hóa của compiler
    }
}

// Hàm khởi tạo bộ hẹn giờ APIC
void apic_timer_init(uint32_t ticks_per_second) {
    // 1. Thiết lập chế độ chia tần số (divider)
    // 0x3 tương ứng với chia tần số 16
    apic_write(APIC_REG_TIMER_DIV_PHYS, 0x3);

    // 2. Đặt thanh ghi EOI ban đầu
    apic_eoi();

    // 3. Cấu hình thanh ghi LVT Timer cho chế độ one-shot để đo tần số
    apic_write(APIC_REG_LVT_TIMER_PHYS, IRQ_TIMER);

    // 4. Đặt bộ đếm APIC timer về -1
    apic_write(APIC_REG_TIMER_INITCNT_PHYS, 0xFFFFFFFF);

    // 5. Chờ một khoảng thời gian xác định (sử dụng PIT hoặc một bộ hẹn giờ khác)
    // Ở đây giả định rằng bạn đã có hàm delay_ms(ms) để chờ
    delay_ms(100); // Chờ 100 ms

    // 6. Đọc giá trị bộ đếm hiện tại
    uint32_t current_count = apic_read(APIC_REG_TIMER_CURRCNT_PHYS);

    // 7. Tính toán tần số APIC timer
    // Giả sử bạn biết thời gian đã chờ (100 ms)
    // ticks_per_second = (0xFFFFFFFF - current_count) / (time_seconds)
    uint32_t ticks = 0xFFFFFFFF - current_count;
    // Vì delay_ms(100) = 0.1 giây
    // Tính tần số APIC timer = ticks / 0.1 = ticks * 10
    uint32_t apic_frequency = ticks * 10;

    // 8. Đặt lại bộ đếm APIC timer với giá trị phù hợp cho ticks_per_second
    uint32_t initial_count = apic_frequency / ticks_per_second;
    apic_write(APIC_REG_TIMER_INITCNT_PHYS, initial_count);

    // 9. Đặt chế độ periodic cho bộ hẹn giờ APIC
    apic_write(APIC_REG_LVT_TIMER_PHYS, IRQ_TIMER | (1 << 17) | APIC_LVT_TIMER_PERIODIC);
}
