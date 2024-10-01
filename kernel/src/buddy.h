#ifndef BUDDY_H
#define BUDDY_H

#include <stddef.h>

#define MIN_ORDER 12 // 4KB
#define MAX_ORDER 33 // 1MB
#define PAGE_SIZE 4096 // 4KB
#define ALIGNMENT 16

struct free_block {
    struct free_block *next;
};

struct alloc_header {
    int order;
};

void buddy_init();
void buddy_print_free_memory();
void* buddy_alloc(size_t size);
void buddy_free(void *ptr);

#endif // BUDDY_H
