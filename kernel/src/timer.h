// timer.h
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Khởi tạo timer với tần số (Hz)
void timer_init(uint32_t frequency);

// Hàm gửi End Of Interrupt (EOI) cho bộ định thời, cần triển khai theo hệ thống của bạn
void send_eoi();

#endif // TIMER_H
