#include "buddy.h"
#include <stddef.h>
#include "limine.h"
#include <stdbool.h>
#include <spinlock.h>

struct free_block *free_lists[MAX_ORDER - MIN_ORDER + 1];

extern volatile struct limine_memmap_request memmap_request;

spinlock_t buddy_lock;

extern uintptr_t hhdm_offset; 

void buddy_init() {
    for (int i = 0; i < (MAX_ORDER - MIN_ORDER + 1); i++) {
        free_lists[i] = NULL;
    }

    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uintptr_t base = entry->base;
            uintptr_t length = entry->length;

            uintptr_t aligned_base = (base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
            uintptr_t aligned_length = length - (aligned_base - base);
            aligned_length &= ~(PAGE_SIZE - 1);

            for (uintptr_t addr = aligned_base; addr < aligned_base + aligned_length; addr += PAGE_SIZE) {
                struct free_block *block = (struct free_block *)addr;
                block->next = free_lists[0];
                free_lists[0] = block;
            }
        }
    }
}

int find_order(size_t size) {
    size_t total_size = size + sizeof(struct free_block); // Bao gồm metadata nếu cần
    int order = MIN_ORDER;
    while((1UL << order) < total_size && order <= MAX_ORDER) {
        order++;
    }
    return order;
}

struct free_block* split_block(int order) {
    if(order >= (MAX_ORDER - MIN_ORDER))
        return NULL;

    if (free_lists[order] == NULL) {
        struct free_block *buddy = split_block(order + 1);
        if (buddy = NULL)
            return NULL;

        size_t block_size = 1UL << (order + MIN_ORDER);
        struct free_block *buddy1 = buddy;
        struct free_block *buddy2 = (struct free_block *)((uintptr_t)buddy + block_size);

        buddy1->next = buddy2;
        buddy2->next = free_lists[order];
        free_lists[order] = buddy1;
    }

    struct free_block *block = free_lists[order];
    free_lists[order] = block->next;
    return block;
}

void* buddy_alloc(size_t size) {
    spin_lock(&buddy_lock);
    
    int order = find_order(size);
    if(order > MAX_ORDER) {
        spin_unlock(&buddy_lock);
        return NULL; // Yêu cầu vượt quá kích thước tối đa
    }
    
    int list_index = order - MIN_ORDER;
    if(list_index < 0 || list_index >= (MAX_ORDER - MIN_ORDER + 1)) {
        spin_unlock(&buddy_lock);
        return NULL; // Cấp độ không hợp lệ
    }
    
    struct free_block *block = split_block(list_index);
    if(block == NULL) {
        spin_unlock(&buddy_lock);
        return NULL; // Không đủ bộ nhớ
    }
    
    spin_unlock(&buddy_lock);
    return (void *)block;
}

void buddy_free(void *ptr, size_t size) {
    spin_lock(&buddy_lock);
    
    int order = find_order(size) - MIN_ORDER;
    if(order < 0 || order >= (MAX_ORDER - MIN_ORDER + 1)) {
        // Giải phóng không hợp lệ
        spin_unlock(&buddy_lock);
        return;
    }
    
    struct free_block *block = (struct free_block *)ptr;
    
    while(order < (MAX_ORDER - MIN_ORDER)) {
        size_t block_size = 1UL << (order + MIN_ORDER);
        uintptr_t buddy_addr = ((uintptr_t)block) ^ block_size;
        struct free_block *buddy = (struct free_block *)buddy_addr;
        
        // Tìm buddy trong free list
        struct free_block **current = &free_lists[order];
        bool buddy_found = false;
        while(*current) {
            if(*current == buddy) {
                // Buddy tìm thấy, hợp nhất
                *current = buddy->next;
                buddy_found = true;
                break;
            }
            current = &(*current)->next;
        }
        
        if(!buddy_found)
            break; // Không tìm thấy buddy, không thể hợp nhất
        
        // Chọn địa chỉ thấp hơn
        if(block > buddy)
            block = buddy;
        
        // Tăng cấp độ để hợp nhất
        order++;
    }
    
    // Thêm khối vào free list
    block->next = free_lists[order];
    free_lists[order] = block;
    
    spin_unlock(&buddy_lock);
}
