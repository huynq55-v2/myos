// process.c
#include "process.h"
#include "elf_loader.h"
#include "context_switcher.h"
#include "paging.h"
#include "klibc.h"
#include "graphics.h"

#include "bitmap_allocator.h"

#include <stddef.h>
#include "config.h"

// Các hàng đợi và biến toàn cục
static process_t *ready_queue_head = NULL;
static process_t *ready_queue_tail = NULL;
uint64_t current_pid = 1;
process_t *current_process = NULL;

// Hàm tìm tiến trình theo PID
process_t* get_process_by_pid(uint64_t pid) {
    // Duyệt qua hàng đợi sẵn sàng
    process_t *proc = ready_queue_head;
    while (proc) {
        if (proc->pid == pid) return proc;
        proc = proc->next;
    }
    // Có thể mở rộng tìm kiếm trong danh sách khác nếu có
    return NULL;
}

void process_enqueue(process_t *proc) {
    if (!proc) return;

    proc->next = NULL;
    if (ready_queue_tail) {
        ready_queue_tail->next = proc;
    } else {
        ready_queue_head = proc;
    }
    ready_queue_tail = proc;
}

process_t *process_dequeue() {
    if (!ready_queue_head) return NULL;

    process_t *proc = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    if (!ready_queue_head) {
        ready_queue_tail = NULL;
    }
    proc->next = NULL;
    return proc;
}

// Hàm tạo một tiến trình mới từ ELF binary
process_t *process_create(uint8_t *elf_start, uint8_t *elf_end, uint64_t parent_pid)
{
    if (!elf_start || !elf_end || elf_start >= elf_end) {
        kprintf("Process Manager: Invalid ELF binary\n");
        return NULL;
    }

    process_t *proc = (process_t *)PHYS_TO_VIRT(allocate_memory_bytes(sizeof(process_t)));
    if (!proc) {
        kprintf("Process Manager: Failed to allocate memory for process\n");
        return NULL;
    }

    memset(proc, 0, sizeof(process_t));

    proc->pid = current_pid++;
    proc->parent_pid = parent_pid;
    proc->state = PROCESS_STATE_READY;

    proc->page_table = (uint64_t)create_user_page_table();
    if (!proc->page_table) {
        kprintf("Process Manager: Failed to create page table\n");
        free_memory_bytes(VIRT_TO_PHYS(proc), sizeof(process_t));
        return NULL;
    }

    uint64_t entry_point = elf_load(proc->page_table, elf_start, elf_end);
    if (!entry_point) {
        kprintf("Process Manager: Failed to load ELF binary\n");
        free_memory_bytes(VIRT_TO_PHYS(proc), sizeof(process_t));
        // Cần giải phóng các tài nguyên khác
        return NULL;
    }

    // Allocate and map user stack
    uint64_t user_stack_phys = allocate_physical_block();
    if (!user_stack_phys) {
        kprintf("Process Manager: Failed to allocate user stack\n");
        free_memory_bytes(VIRT_TO_PHYS(proc), sizeof(process_t));
        // Cần giải phóng các tài nguyên khác
        return NULL;
    }

    uint64_t user_stack_virt = 0x7FFFFFFF0000;
    if (!map_memory(proc->page_table, user_stack_virt - BLOCK_SIZE, user_stack_phys, BLOCK_SIZE, PAGING_PAGE_PRESENT | PAGING_PAGE_RW | PAGING_PAGE_USER)) {
        kprintf("Process Manager: Failed to map user stack\n");
        free_memory_bytes(VIRT_TO_PHYS(proc), sizeof(process_t));
        // Giải phóng các tài nguyên khác
        return NULL;
    }

    proc->context.rip = entry_point;
    proc->context.rsp = user_stack_virt - 16; // 16-byte aligned

    // Liên kết với tiến trình cha
    if (parent_pid != 0) {
        process_t *parent = get_process_by_pid(parent_pid);
        if (parent) {
            proc->sibling = parent->children;
            parent->children = proc;
        }
    }

    process_enqueue(proc);
    kprintf("Process Manager: Created process PID=%llu\n", proc->pid);
    return proc;
}

// Hàm chạy tiến trình đầu tiên
void process_run()
{
    process_t *proc = process_dequeue();
    if (!proc)
    {
        kprintf("Process Manager: No process to run\n");
        return;
    }
    proc->state = PROCESS_STATE_RUNNING;
    current_process = proc;
    switch_to_user_space(proc->context.rip, proc->context.rsp, proc->page_table);
}

// Hàm xử lý kết thúc tiến trình
void process_exit(int exit_code)
{
    current_process->state = PROCESS_STATE_ZOMBIE;
    current_process->exit_code = exit_code;
    // Thông báo cho tiến trình cha nếu cần
    // Chuyển sang tiến trình khác
    process_run();
}

// // Implement sys_fork
// int sys_fork()
// {
//     if (!current_process) {
//         return -1; // Không có tiến trình hiện tại
//     }

//     // Tạo tiến trình con bằng cách gọi process_create với cùng ELF binary
//     // Giả sử bạn có cách lấy ELF binary từ tiến trình cha
//     uint8_t *elf_start = ...; // Cần implement
//     uint8_t *elf_end = ...;   // Cần implement

//     process_t *child = process_create(elf_start, elf_end, current_process->pid);
//     if (!child) {
//         return -1;
//     }

//     // Sao chép ngữ cảnh CPU
//     child->context = current_process->context;
//     child->context.rip = child->context.rip; // Có thể cần điều chỉnh

//     // Trả về PID của tiến trình con cho tiến trình cha
//     return child->pid;
// }

// // Implement sys_execve
// int sys_execve(const char *pathname, char *const argv[], char *const envp[])
// {
//     // Tải ELF binary mới từ pathname
//     uint8_t *elf_start, *elf_end;
//     if (!load_elf_from_path(pathname, &elf_start, &elf_end)) {
//         return -1; // Lỗi khi tải ELF
//     }

//     // Giải phóng không gian địa chỉ hiện tại
//     // Giả sử bạn có hàm để giải phóng không gian
//     free_address_space(current_process->page_table);

//     // Tạo lại page table mới
//     current_process->page_table = (uint64_t)create_user_page_table();
//     if (!current_process->page_table) {
//         return -1;
//     }

//     // Tải ELF mới
//     uint64_t entry_point = elf_load(current_process->page_table, elf_start, elf_end);
//     if (!entry_point) {
//         return -1;
//     }

//     // Map lại stack
//     uint64_t user_stack_phys = allocate_physical_block();
//     if (!user_stack_phys) {
//         return -1;
//     }

//     uint64_t user_stack_virt = 0x7FFFFFFF0000;
//     if (!map_memory(current_process->page_table, user_stack_virt - BLOCK_SIZE, user_stack_phys, BLOCK_SIZE, PAGING_PAGE_PRESENT | PAGING_PAGE_RW | PAGING_PAGE_USER)) {
//         return -1;
//     }

//     current_process->context.rip = entry_point;
//     current_process->context.rsp = user_stack_virt - 16; // 16-byte aligned

//     // Có thể cần xử lý argv và envp
//     // ...

//     // Chạy tiến trình mới
//     switch_to_user_space(current_process->context.rip, current_process->context.rsp, current_process->page_table);

//     return 0;
// }

// // Implement sys_waitpid
// pid_t sys_waitpid(pid_t pid, int *status, int options)
// {
//     // Kiểm tra xem pid có phải là con của tiến trình hiện tại không
//     process_t *child = get_process_by_pid(pid);
//     if (!child || child->parent_pid != current_process->pid) {
//         return -1;
//     }

//     // Nếu tiến trình con đã kết thúc
//     if (child->state == PROCESS_STATE_ZOMBIE) {
//         if (status) {
//             *status = child->exit_code;
//         }
//         // Giải phóng tài nguyên của tiến trình con
//         // ...
//         return pid;
//     }

//     // Nếu chưa kết thúc, tiến trình cha sẽ bị block
//     current_process->state = PROCESS_STATE_BLOCKED;
//     // Thêm cơ chế để tiến trình cha được đánh thức khi con kết thúc
//     // ...

//     // Chạy tiến trình khác
//     process_run();

//     return pid;
// }
