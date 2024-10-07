// context_switcher.c
#include "context_switcher.h"
#include "paging.h"
#include "gdt.h"
#include "klibc.h"
#include "graphics.h"

extern void _save_context(cpu_context_t *context);
extern void _switch_to_context(cpu_context_t *new_context);

// Hàm lưu ngữ cảnh của tiến trình hiện tại
void save_context(cpu_context_t *context) {
    _save_context(context);
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
