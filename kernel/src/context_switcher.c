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

void switch_to_user_space(cpu_context_t *context) {
    _switch_to_context(context);
}
