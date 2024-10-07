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
uint64_t copy_page_table(uint64_t parent_pml4_phys) {
    uint64_t child_pml4_phys = 0;
    uint64_t child_pdpt_phys = 0;
    uint64_t child_pd_phys = 0;
    uint64_t child_pt_phys = 0;
    uint64_t *parent_pml4_virt = NULL;
    uint64_t *child_pml4_virt = NULL;

    // Allocate a new PML4 for the child
    child_pml4_phys = allocate_physical_block();
    if (!child_pml4_phys) {
        kprintf("copy_page_table: Failed to allocate PML4\n");
        return 0;
    }

    // Convert physical addresses to virtual addresses
    parent_pml4_virt = PHYS_TO_VIRT(parent_pml4_phys);
    child_pml4_virt = PHYS_TO_VIRT(child_pml4_phys);

    // Zero out the new PML4
    memset(child_pml4_virt, 0, PAGE_SIZE);

    // Copy kernel space mappings from the parent to the child
    for (int i = 256; i < 512; i++) {
        child_pml4_virt[i] = parent_pml4_virt[i];
    }

    // Copy user space mappings from the parent to the child
    for (int i = 0; i < 256; i++) {
        if (parent_pml4_virt[i] & PAGING_PAGE_PRESENT) {
            // Allocate a new PDPT for the child
            child_pdpt_phys = allocate_physical_block();
            if (!child_pdpt_phys) {
                kprintf("copy_page_table: Failed to allocate PDPT\n");
                goto cleanup_pml4;
            }

            uint64_t *parent_pdpt_virt = PHYS_TO_VIRT(parent_pml4_virt[i] & 0xFFFFFFFFFFFFF000);
            uint64_t *child_pdpt_virt = PHYS_TO_VIRT(child_pdpt_phys);
            memset(child_pdpt_virt, 0, PAGE_SIZE);

            // Set the PDPT entry in the child's PML4
            child_pml4_virt[i] = (child_pdpt_phys & 0xFFFFFFFFFFFFF000) | (parent_pml4_virt[i] & 0xFFF);

            for (int j = 0; j < 512; j++) {
                if (parent_pdpt_virt[j] & PAGING_PAGE_PRESENT) {
                    // Allocate a new PD for the child
                    child_pd_phys = allocate_physical_block();
                    if (!child_pd_phys) {
                        kprintf("copy_page_table: Failed to allocate PD\n");
                        goto cleanup_pdpt;
                    }

                    uint64_t *parent_pd_virt = PHYS_TO_VIRT(parent_pdpt_virt[j] & 0xFFFFFFFFFFFFF000);
                    uint64_t *child_pd_virt = PHYS_TO_VIRT(child_pd_phys);
                    memset(child_pd_virt, 0, PAGE_SIZE);

                    // Set the PD entry in the child's PDPT
                    child_pdpt_virt[j] = (child_pd_phys & 0xFFFFFFFFFFFFF000) | (parent_pdpt_virt[j] & 0xFFF);

                    for (int k = 0; k < 512; k++) {
                        if (parent_pd_virt[k] & PAGING_PAGE_PRESENT) {
                            // Allocate a new PT for the child
                            child_pt_phys = allocate_physical_block();
                            if (!child_pt_phys) {
                                kprintf("copy_page_table: Failed to allocate PT\n");
                                goto cleanup_pd;
                            }

                            uint64_t *parent_pt_virt = PHYS_TO_VIRT(parent_pd_virt[k] & 0xFFFFFFFFFFFFF000);
                            uint64_t *child_pt_virt = PHYS_TO_VIRT(child_pt_phys);
                            memset(child_pt_virt, 0, PAGE_SIZE);

                            // Set the PT entry in the child's PD
                            child_pd_virt[k] = (child_pt_phys & 0xFFFFFFFFFFFFF000) | (parent_pd_virt[k] & 0xFFF);

                            for (int l = 0; l < 512; l++) {
                                if (parent_pt_virt[l] & PAGING_PAGE_PRESENT) {
                                    uint64_t parent_page_phys = parent_pt_virt[l] & 0xFFFFFFFFFFFFF000;

                                    // Allocate a new physical page for the child
                                    uint64_t child_page_phys = allocate_physical_block();
                                    if (!child_page_phys) {
                                        kprintf("copy_page_table: Failed to allocate page\n");
                                        goto cleanup_pt;
                                    }

                                    // Copy the content from parent to child
                                    memcpy(PHYS_TO_VIRT(child_page_phys), PHYS_TO_VIRT(parent_page_phys), PAGE_SIZE);

                                    // Set the page entry in the child's PT
                                    child_pt_virt[l] = (child_page_phys & 0xFFFFFFFFFFFFF000) | (parent_pt_virt[l] & 0xFFF);
                                }
                            }
                            // Reset child_pt_phys after use
                            child_pt_phys = 0;
                            continue;
                        }
                    cleanup_pt:
                        if (child_pt_phys) {
                            free_physical_block(child_pt_phys);
                            child_pt_phys = 0;
                        }
                    }
                    // Reset child_pd_phys after use
                    child_pd_phys = 0;
                    continue;
                }
            cleanup_pd:
                if (child_pd_phys) {
                    free_physical_block(child_pd_phys);
                    child_pd_phys = 0;
                }
            }
            // Reset child_pdpt_phys after use
            child_pdpt_phys = 0;
            continue;
        }
    cleanup_pdpt:
        if (child_pdpt_phys) {
            free_physical_block(child_pdpt_phys);
            child_pdpt_phys = 0;
        }
    }

    return child_pml4_phys;

cleanup_pml4:
    free_physical_block(child_pml4_phys);
    return 0;
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
