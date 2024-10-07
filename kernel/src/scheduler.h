// scheduler.h
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

// Khởi tạo bộ lập lịch
void scheduler_init();

// Trình xử lý ngắt timer
void scheduler_handle_timer_interrupt();

// Vòng lặp chính của scheduler (nếu cần)
void scheduler_run();

#endif // SCHEDULER_H
