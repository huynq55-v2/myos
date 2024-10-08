// process.h
#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include "memory_manager.h"
#include "paging.h"

// Định nghĩa PID
typedef int pid_t;

// Định nghĩa trạng thái của tiến trình
typedef enum {
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_READY,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_TERMINATED,
    PROCESS_STATE_ZOMBIE // Trạng thái zombie cho waitpid
} process_state_t;

// Cấu trúc ngữ cảnh CPU
typedef struct cpu_context {
    uint64_t rax;      // 0x00
    uint64_t rbx;      // 0x08
    uint64_t rcx;      // 0x10
    uint64_t rdx;      // 0x18
    uint64_t rsi;      // 0x20
    uint64_t rdi;      // 0x28
    uint64_t rsp;      // 0x30
    uint64_t rbp;      // 0x38
    uint64_t r8;       // 0x40
    uint64_t r9;       // 0x48
    uint64_t r10;      // 0x50
    uint64_t r11;      // 0x58
    uint64_t r12;      // 0x60
    uint64_t r13;      // 0x68
    uint64_t r14;      // 0x70
    uint64_t r15;      // 0x78
    uint64_t rip;      // 0x80
    uint64_t rflags;   // 0x88
    uint64_t cr3;      // 0x90
} cpu_context_t;

// Cấu trúc cho một tiến trình
typedef struct process {
    uint64_t pid;                      // ID của tiến trình
    uint64_t parent_pid;               // PID của tiến trình cha
    uint64_t page_table;               // Địa chỉ vật lý của page table
    process_state_t state;             // Trạng thái của tiến trình
    cpu_context_t context;             // Ngữ cảnh CPU
    struct process *next;              // Con trỏ đến tiến trình kế tiếp (dùng trong hàng đợi)
    struct process *children;          // Con trỏ đến danh sách con tiến trình
    struct process *sibling;           // Con trỏ đến tiến trình cùng cấp
    int exit_code;                     // Mã kết thúc của tiến trình
} process_t;

// Hàm tạo một tiến trình mới từ ELF binary
process_t* process_create(uint8_t *elf_start, uint8_t *elf_end, uint64_t parent_pid);

// Hàm chạy tiến trình đầu tiên
void process_run();

// Hàm thêm tiến trình vào hàng đợi sẵn sàng
void process_enqueue(process_t *proc);

// Hàm lấy tiến trình tiếp theo từ hàng đợi sẵn sàng
process_t* process_dequeue();

// Hệ thống call fork
pid_t sys_fork();
pid_t sys_waitpid(pid_t pid, int *status);

// Hàm xử lý kết thúc tiến trình
void process_exit(int exit_code);

// Hàm tìm tiến trình theo PID
process_t* get_process_by_pid(uint64_t pid);

// Khai báo các hàm sao chép bảng trang
uint64_t copy_page_table(uint64_t parent_pml4_phys_addr);
void free_page_table(uint64_t pml4_phys_addr);

#endif // PROCESS_H
