#ifndef BUDDY_H
#define BUDDY_H

#include <stddef.h>

#define MIN_ORDER 12 // 4KB
#define MAX_ORDER 20 // 1MB
#define PAGE_SIZE 4096 // 4KB

struct free_block {
    struct free_block *next;
};

void buddy_init();
void* buddy_alloc(size_t size);
void buddy_free(void *ptr, size_t size);

#endif // BUDDY_H
