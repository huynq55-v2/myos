// apic.h
#ifndef APIC_H
#define APIC_H

#include <stdint.h>
#include <stdbool.h>

// Định nghĩa offset cho High Half Direct Mapping (HHDM)
#define HHDM_OFFSET 0xFFFF800000000000

// Macro để chuyển đổi từ địa chỉ vật lý sang địa chỉ ảo
#define PHYS_TO_VIRT(x) ((void *)((uintptr_t)(x) + HHDM_OFFSET))

// Macro để chuyển đổi từ địa chỉ ảo sang địa chỉ vật lý
#define VIRT_TO_PHYS(x) ((void *)((uintptr_t)(x) - HHDM_OFFSET))

// Địa chỉ cơ sở của Local APIC (vật lý)
#define APIC_BASE_PHYS 0xFEE00000

// Các thanh ghi APIC (vật lý)
#define APIC_REG_TIMER_DIV_PHYS       (APIC_BASE_PHYS + 0x3E0)
#define APIC_REG_TIMER_INITCNT_PHYS   (APIC_BASE_PHYS + 0x380)
#define APIC_REG_TIMER_CURRCNT_PHYS   (APIC_BASE_PHYS + 0x390)
#define APIC_REG_LVT_TIMER_PHYS       (APIC_BASE_PHYS + 0x320)
#define APIC_REG_SPURIOUS_PHYS        (APIC_BASE_PHYS + 0xF0)
#define APIC_REG_EOI_PHYS             (APIC_BASE_PHYS + 0xB0)

// Các hằng số APIC
#define APIC_SW_ENABLE           0x100
#define APIC_DISABLE             0x10000
#define APIC_LVT_TIMER_PERIODIC  0x20000
#define APIC_LVT_INT_MASKED      0x00010000

// Hằng số ngắt
#define IRQ_TIMER                32
#define IRQ_SPURIOUS             39

// Các chế độ bộ hẹn giờ
#define TIMER_MODE_ONESHOT       0
#define TIMER_MODE_PERIODIC      1
#define TIMER_MODE_TSC_DEADLINE  2

#define APIC_DEFAULT_FREQUENCY 10000000

#ifdef __cplusplus
extern "C" {
#endif

// Hàm để ghi vào một thanh ghi APIC (sử dụng địa chỉ ảo)
static inline void apic_write(uint32_t address_phys, uint32_t value);

// Hàm để đọc từ một thanh ghi APIC (sử dụng địa chỉ ảo)
static inline uint32_t apic_read(uint32_t address_phys);

// Hàm gửi tín hiệu EOI (End Of Interrupt)
void apic_eoi();

// ISR cho bộ hẹn giờ APIC
void apic_timer_isr();

// Hàm khởi tạo bộ hẹn giờ APIC
void apic_timer_init(uint32_t ticks_per_second);

#ifdef __cplusplus
}
#endif

#endif // APIC_H
