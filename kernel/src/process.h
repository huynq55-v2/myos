// process.h

#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} process_state_t;

typedef struct process {
    uint64_t pid;
    uint64_t entry_point;
    process_state_t state;
    struct process *next;
    void *page_table; // Con trỏ tới bảng trang của tiến trình
    // Thêm các trường cần thiết khác (ví dụ: CPU state)
} process_t;

// Khởi tạo quản lý tiến trình
void process_init();

// Tạo tiến trình mới từ ELF
process_t* process_create(const uint8_t *elf_data, size_t size);

// Lịch trình tiến trình tiếp theo
void schedule();

// Tiến trình hiện tại
process_t* current_process();

// Kết thúc tiến trình hiện tại
void process_terminate();

#endif // PROCESS_H
