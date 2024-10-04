// context_switcher.c
#include "context_switcher.h"
#include "paging.h"
#include "gdt.h"
#include "klibc.h"
#include "graphics.h"

// Hàm lưu ngữ cảnh của tiến trình hiện tại
void save_context(process_t *proc) {
    asm volatile (
        "movq %%rsp, %0\n\t"
        "movq %%rbp, %1\n\t"
        "movq %%rbx, %2\n\t"
        "movq %%r12, %3\n\t"
        "movq %%r13, %4\n\t"
        "movq %%r14, %5\n\t"
        "movq %%r15, %6\n\t"
        "leaq 1f(%%rip), %%rax\n\t"
        "movq %%rax, %7\n\t"
        "1:\n\t"
        : "=m"(proc->context.rsp),
          "=m"(proc->context.rbp),
          "=m"(proc->context.rbx),
          "=m"(proc->context.r12),
          "=m"(proc->context.r13),
          "=m"(proc->context.r14),
          "=m"(proc->context.r15),
          "=m"(proc->context.rip)
        :
        : "rax"
    );
}

// Hàm tải ngữ cảnh của tiến trình tiếp theo
void load_context(process_t *proc) {
    asm volatile (
        "movq %0, %%rsp\n\t"
        "movq %1, %%rbp\n\t"
        "movq %2, %%rbx\n\t"
        "movq %3, %%r12\n\t"
        "movq %4, %%r13\n\t"
        "movq %5, %%r14\n\t"
        "movq %6, %%r15\n\t"
        "jmp *%7\n\t"
        :
        : "m"(proc->context.rsp),
          "m"(proc->context.rbp),
          "m"(proc->context.rbx),
          "m"(proc->context.r12),
          "m"(proc->context.r13),
          "m"(proc->context.r14),
          "m"(proc->context.r15),
          "m"(proc->context.rip)
    );
}

void switch_to_user_space(uint64_t entry_point, uint64_t stack_pointer, uint64_t page_table_phys) {
    // Cập nhật CR3 để chuyển đổi page table
    switch_page_table((void*)page_table_phys);

    asm volatile (
        "mov %0, %%rsp\n\t"          // Thiết lập ngăn xếp
        "mov %1, %%rbp\n\t"          // Thiết lập rbp
        "pushq $0x23\n\t"            // SS: User Data Segment (selector 0x23)
        "pushq %%rsp\n\t"            // RSP
        "pushfq\n\t"                 // RFLAGS
        "pushq $0x1B\n\t"            // CS: User Code Segment (selector 0x1B)
        "pushq %2\n\t"               // RIP: entry point
        "iretq\n\t"                  // Trở về user space
        :
        : "r"(stack_pointer),
          "r"(stack_pointer),
          "r"(entry_point)
        : "memory"
    );
}
