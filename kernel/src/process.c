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

static process_t *ready_queue_head = NULL;
static process_t *ready_queue_tail = NULL;
uint64_t current_pid = 1;

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
process_t *process_create(uint8_t *elf_start, uint8_t *elf_end)
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
        // Also free page table and any allocated resources
        return NULL;
    }

    // Allocate and map user stack
    uint64_t user_stack_phys = allocate_physical_block();
    if (!user_stack_phys) {
        kprintf("Process Manager: Failed to allocate user stack\n");
        free_memory_bytes(VIRT_TO_PHYS(proc), sizeof(process_t));
        // Also free page table and any allocated resources
        return NULL;
    }

    uint64_t user_stack_virt = 0x7FFFFFFF0000;
    if (!map_memory(proc->page_table, user_stack_virt - BLOCK_SIZE, user_stack_phys, BLOCK_SIZE, PAGING_PAGE_PRESENT | PAGING_PAGE_RW | PAGING_PAGE_USER)) {
        kprintf("Process Manager: Failed to map user stack\n");
        free_memory_bytes(VIRT_TO_PHYS(proc), sizeof(process_t));
        // Free other resources
        return NULL;
    }

    proc->context.rip = entry_point;
    proc->context.rsp = user_stack_virt - 16; // 16-byte aligned

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
    switch_to_user_space(proc->context.rip, proc->context.rsp, proc->page_table);
}
