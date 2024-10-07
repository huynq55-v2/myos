// scheduler.c
#include "scheduler.h"
#include "timer.h"
#include "interrupts.h"
#include "context_switcher.h"
#include "klibc.h"

// Biến toàn cục tiến trình hiện tại
extern process_t *current_process;

// Hàm xử lý ngắt timer trong C
void timer_interrupt_handler_c(uint64_t vector_number, isr_stack_t *stack) {
    // Lưu ngữ cảnh của tiến trình hiện tại
    if (current_process) {
        save_context(&current_process->context);
        
        // Đặt trạng thái của tiến trình hiện tại về READY nếu nó vẫn đang chạy
        if (current_process->state == PROCESS_STATE_RUNNING) {
            current_process->state = PROCESS_STATE_READY;
            process_enqueue(current_process);
        }
    }

    // Lấy tiến trình tiếp theo từ hàng đợi sẵn sàng
    process_t *next_proc = process_dequeue();
    if (next_proc) {
        next_proc->state = PROCESS_STATE_RUNNING;
        current_process = next_proc;
        switch_to_user_space(&next_proc->context);
    } else {
        // Nếu không có tiến trình nào sẵn sàng, thực hiện idle
        while (1); // Hoặc triển khai một vòng lặp idle
    }

    // Gửi End Of Interrupt (EOI) cho APIC
    send_eoi();
}
extern uint8_t hello_user_elf_start[];
extern uint8_t hello_user_elf_end[];

// Hàm khởi tạo bộ lập lịch
void scheduler_init() {
    // Đăng ký trình xử lý ngắt timer (vector 32)
    register_interrupt_handler(TIMER_INTERRUPT_VECTOR, timer_interrupt_handler_c);

    // Tạo tiến trình init và đưa vào hàng đợi sẵn sàng
    process_t *init_proc = process_create(hello_user_elf_start, hello_user_elf_end, 0);
    if (!init_proc) {
        kprintf("Scheduler: Failed to create init process\n");
        while (1) {}
    }

    process_enqueue(init_proc);
}

// Vòng lặp chính của scheduler (nếu cần)
void scheduler_run() {
    // Trong Round Robin với ngắt định kỳ, vòng lặp chính có thể chỉ cần idle
    while (1) {
        hcf(); // Hoặc triển khai một vòng lặp idle
    }
}

void scheduler_start() {
    process_t *next_proc = process_dequeue();
    if (next_proc) {
        next_proc->state = PROCESS_STATE_RUNNING;
        current_process = next_proc;
        switch_to_user_space(&next_proc->context);
    } else {
        kprintf("Scheduler: No process to run\n");
    }
}
