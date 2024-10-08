// context_switcher.h
#ifndef CONTEXT_SWITCHER_H
#define CONTEXT_SWITCHER_H

#include "process.h"

// Hàm lưu ngữ cảnh của tiến trình hiện tại
void save_context(cpu_context_t *context);

// Hàm chuyển đổi sang user space
void switch_to_user_space(cpu_context_t *new_context);

#endif // CONTEXT_SWITCHER_H
