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

typedef int pid_t;

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

// ==============================================================================
// Hàm sao chép bảng trang từ tiến trình cha sang tiến trình con
uint64_t copy_page_table(uint64_t parent_pml4_phys_addr) {
    // Tạo một bảng PML4 mới cho tiến trình con
    uint64_t child_pml4_phys_addr = allocate_physical_block();
    if (!child_pml4_phys_addr) {
        kprintf("copy_page_table: Không thể cấp phát PML4 cho tiến trình con\n");
        return 0;
    }

    // Chuyển địa chỉ vật lý sang địa chỉ ảo để thao tác
    uint64_t *parent_pml4 = (uint64_t *)PHYS_TO_VIRT(parent_pml4_phys_addr);
    uint64_t *child_pml4 = (uint64_t *)PHYS_TO_VIRT(child_pml4_phys_addr);

    // Sao chép nội dung của PML4
    memset(child_pml4, 0, BLOCK_SIZE);

    for (int i = 0; i < 512; i++) {
        if (parent_pml4[i] & PAGING_PAGE_PRESENT) {
            // Lấy địa chỉ của bảng PDPT
            uint64_t parent_pdpt_phys_addr = parent_pml4[i] & ~0xFFF;
            uint64_t child_pdpt_phys_addr = allocate_physical_block();
            if (!child_pdpt_phys_addr) {
                kprintf("copy_page_table: Không thể cấp phát PDPT\n");
                // Cần giải phóng các tài nguyên đã cấp phát
                free_page_table(child_pml4_phys_addr);
                return 0;
            }

            // Liên kết PDPT vào PML4 của tiến trình con
            child_pml4[i] = child_pdpt_phys_addr | (parent_pml4[i] & 0xFFF);

            // Sao chép PDPT
            if (!copy_pdpt(parent_pdpt_phys_addr, child_pdpt_phys_addr)) {
                // Giải phóng các tài nguyên đã cấp phát
                free_page_table(child_pml4_phys_addr);
                return 0;
            }
        }
    }

    return child_pml4_phys_addr;
}

// Hàm sao chép PDPT
bool copy_pdpt(uint64_t parent_pdpt_phys_addr, uint64_t child_pdpt_phys_addr) {
    uint64_t *parent_pdpt = (uint64_t *)PHYS_TO_VIRT(parent_pdpt_phys_addr);
    uint64_t *child_pdpt = (uint64_t *)PHYS_TO_VIRT(child_pdpt_phys_addr);

    memset(child_pdpt, 0, BLOCK_SIZE);

    for (int i = 0; i < 512; i++) {
        if (parent_pdpt[i] & PAGING_PAGE_PRESENT) {
            uint64_t parent_pdt_phys_addr = parent_pdpt[i] & ~0xFFF;
            uint64_t child_pdt_phys_addr = allocate_physical_block();
            if (!child_pdt_phys_addr) {
                kprintf("copy_pdpt: Không thể cấp phát PDT\n");
                return false;
            }

            child_pdpt[i] = child_pdt_phys_addr | (parent_pdpt[i] & 0xFFF);

            if (!copy_pdt(parent_pdt_phys_addr, child_pdt_phys_addr)) {
                return false;
            }
        }
    }

    return true;
}

// Hàm sao chép PDT
bool copy_pdt(uint64_t parent_pdt_phys_addr, uint64_t child_pdt_phys_addr) {
    uint64_t *parent_pdt = (uint64_t *)PHYS_TO_VIRT(parent_pdt_phys_addr);
    uint64_t *child_pdt = (uint64_t *)PHYS_TO_VIRT(child_pdt_phys_addr);

    memset(child_pdt, 0, BLOCK_SIZE);

    for (int i = 0; i < 512; i++) {
        if (parent_pdt[i] & PAGING_PAGE_PRESENT) {
            uint64_t parent_pt_phys_addr = parent_pdt[i] & ~0xFFF;
            uint64_t child_pt_phys_addr = allocate_physical_block();
            if (!child_pt_phys_addr) {
                kprintf("copy_pdt: Không thể cấp phát PT\n");
                return false;
            }

            child_pdt[i] = child_pt_phys_addr | (parent_pdt[i] & 0xFFF);

            if (!copy_pt(parent_pt_phys_addr, child_pt_phys_addr)) {
                return false;
            }
        }
    }

    return true;
}

// Hàm sao chép PT (Page Table)
bool copy_pt(uint64_t parent_pt_phys_addr, uint64_t child_pt_phys_addr) {
    uint64_t *parent_pt = (uint64_t *)PHYS_TO_VIRT(parent_pt_phys_addr);
    uint64_t *child_pt = (uint64_t *)PHYS_TO_VIRT(child_pt_phys_addr);

    memset(child_pt, 0, BLOCK_SIZE);

    for (int i = 0; i < 512; i++) {
        if (parent_pt[i] & PAGING_PAGE_PRESENT) {
            uint64_t parent_page_phys_addr = parent_pt[i] & ~0xFFF;

            // Cấp phát một trang vật lý mới cho tiến trình con
            uint64_t child_page_phys_addr = allocate_physical_block();
            if (!child_page_phys_addr) {
                kprintf("copy_pt: Không thể cấp phát trang vật lý\n");
                return false;
            }

            // Sao chép dữ liệu từ trang của tiến trình cha sang tiến trình con
            memcpy(
                (void *)PHYS_TO_VIRT(child_page_phys_addr),
                (void *)PHYS_TO_VIRT(parent_page_phys_addr),
                BLOCK_SIZE
            );

            // Cập nhật bảng trang của tiến trình con
            child_pt[i] = child_page_phys_addr | (parent_pt[i] & 0xFFF);
        }
    }

    return true;
}

// Hàm giải phóng bảng trang khi gặp lỗi
void free_page_table(uint64_t pml4_phys_addr) {
    // Triển khai hàm này để giải phóng các bảng trang đã cấp phát
    // Khi gặp lỗi trong quá trình sao chép, cần giải phóng các tài nguyên để tránh rò rỉ bộ nhớ
    // ...
}

// ================================================================================

pid_t sys_fork()
{
    if (!current_process) {
        return -1; // No current process
    }

    // Allocate memory for the child process
    process_t *child = (process_t *)PHYS_TO_VIRT(allocate_memory_bytes(sizeof(process_t)));
    if (!child) {
        kprintf("sys_fork: Failed to allocate memory for child process\n");
        return -1;
    }

    // Copy the current process structure to the child
    memcpy(child, current_process, sizeof(process_t));

    // Assign a new PID to the child
    child->pid = current_pid++;
    child->parent_pid = current_process->pid;

    // Copy the parent's page table to the child
    child->page_table = (uint64_t)copy_page_table(current_process->page_table);
    if (!child->page_table) {
        kprintf("sys_fork: Failed to copy page table\n");
        free_memory_bytes(VIRT_TO_PHYS(child), sizeof(process_t));
        return -1;
    }

    // Copy CPU context
    child->context = current_process->context;

    // Set the return value in the child's context to 0
    child->context.rax = 0;

    // Add the child to the ready queue
    process_enqueue(child);

    // Link the child to the parent's list of children
    child->sibling = current_process->children;
    current_process->children = child;

    // Return the child's PID in the parent process
    return child->pid;
}

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
