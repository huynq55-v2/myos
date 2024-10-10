// scheduler.h
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"
#include "idt.h"

// Khởi tạo bộ lập lịch
void scheduler_init();

void timer_interrupt_handler_c();

#endif // SCHEDULER_H
