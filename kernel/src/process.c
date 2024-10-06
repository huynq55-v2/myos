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

// Hàng đợi sẵn sàng (ready queue)
static process_t *ready_queue_head = NULL;
static process_t *ready_queue_tail = NULL;

// Danh sách tiến trình
static process_t *process_list = NULL;

// PID hiện tại
uint64_t current_pid = 1;

// Hàm thêm tiến trình vào hàng đợi sẵn sàng
void process_enqueue(process_t *proc)
{
    proc->next = NULL;
    if (ready_queue_tail)
    {
        ready_queue_tail->next = proc;
    }
    else
    {
        ready_queue_head = proc;
    }
    ready_queue_tail = proc;
}

// Hàm lấy tiến trình tiếp theo từ hàng đợi sẵn sàng
process_t *process_dequeue()
{
    if (!ready_queue_head)
    {
        return NULL;
    }
    process_t *proc = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    if (!ready_queue_head)
    {
        ready_queue_tail = NULL;
    }
    proc->next = NULL;
    return proc;
}

// Hàm khởi tạo hệ thống quản lý tiến trình
void process_manager_init()
{
    // Có thể thêm các thiết lập ban đầu nếu cần
    kprintf("Process Manager Initialized\n");
}

// Hàm tạo một tiến trình mới từ ELF binary
process_t *process_create(uint8_t *elf_start, uint8_t *elf_end)
{
    // Tải ELF vào bộ nhớ và tạo không gian địa chỉ riêng
    process_t *proc = (process_t *)allocate_memory_bytes(sizeof(process_t));
    if (!proc)
    {
        kprintf("Process Manager: Failed to allocate memory for process\n");
        return NULL;
    }
    // convert proc to virtual address
    proc = (process_t *)PHYS_TO_VIRT((uintptr_t)proc);

    memset(proc, 0, sizeof(process_t));

    proc->pid = current_pid++;
    proc->state = PROCESS_STATE_READY;

    // Tạo page table mới cho tiến trình
    proc->page_table = (uint64_t)create_user_page_table();
    if (!proc->page_table)
    {
        kprintf("Process Manager: Failed to create page table\n");
        free_memory_bytes((uint64_t)proc, sizeof(process_t));
        return NULL;
    }

    uint64_t entry_point = elf_load(proc->page_table, elf_start, elf_end);
    if (!entry_point) {
        kprintf("Process Manager: Failed to load ELF binary\n");
        // ... cleanup ...
        return NULL;
    }

    // Thiết lập con trỏ lệnh và ngăn xếp
    proc->context.rip = entry_point; // Địa chỉ entry point đã được ELF loader thiết lập

    // create user stack
    uint64_t user_stack = allocate_physical_block();

    // Bắt đầu ánh xạ từ (0x7FFFFFFFF000 - BLOCK_SIZE)
    map_memory(proc->page_table,
               0x7FFFFFFFF000 - BLOCK_SIZE, // Địa chỉ ảo bắt đầu ánh xạ
               user_stack,                  // Địa chỉ vật lý của block stack
               BLOCK_SIZE,                  // Kích thước của block
               PAGING_PAGE_PRESENT | PAGING_PAGE_RW | PAGING_PAGE_USER);
    proc->context.rsp = 0x7FFFFFFFEFF0;  // Địa chỉ RSP được căn chỉnh theo 16-byte

    // Thêm tiến trình vào danh sách và hàng đợi sẵn sàng
    proc->next = process_list;
    process_list = proc;
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

// Hàm chuyển đổi ngữ cảnh giữa hai tiến trình
void context_switch(process_t *current, process_t *next)
{
    // Lưu trạng thái của tiến trình hiện tại
    save_context(current);

    // Cập nhật trạng thái của tiến trình hiện tại
    current->state = PROCESS_STATE_READY;
    process_enqueue(current);

    // Cập nhật trạng thái của tiến trình tiếp theo
    next->state = PROCESS_STATE_RUNNING;

    // Chuyển đổi ngữ cảnh sang tiến trình tiếp theo
    load_context(next);
}
