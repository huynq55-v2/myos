// context_switcher.h
#ifndef CONTEXT_SWITCHER_H
#define CONTEXT_SWITCHER_H

#include "process.h"

// Hàm lưu ngữ cảnh của tiến trình hiện tại
void save_context(process_t *proc);

// Hàm tải ngữ cảnh của tiến trình tiếp theo
void load_context(process_t *proc);

// Hàm chuyển đổi sang user space
void switch_to_user_space(uint64_t entry_point, uint64_t stack_pointer, uint64_t page_table_phys);

#endif // CONTEXT_SWITCHER_H
