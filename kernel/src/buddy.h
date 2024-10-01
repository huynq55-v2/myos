#ifndef BUDDY_H
#define BUDDY_H

#define MIN_ORDER 12 // 4KB
#define MAX_ORDER 20 // 1MB
#define PAGE_SIZE 4096 // 4KB

struct free_block {
    struct free_block *next;
};

#endif // BUDDY_H
