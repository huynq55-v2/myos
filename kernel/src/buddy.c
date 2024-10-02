#include "buddy.h"
#include <stddef.h>
#include "limine.h"
#include <stdbool.h>
#include "spinlock.h"
#include "paging.h"

struct free_block *free_lists[MAX_ORDER - MIN_ORDER + 1];

extern volatile struct limine_memmap_request memmap_request;
extern uintptr_t hhdm_offset;  // Đảm bảo rằng hhdm_offset đã được khai báo và khởi tạo đúng cách

spinlock_t buddy_lock;

void buddy_init() {
    for (int i = 0; i < (MAX_ORDER - MIN_ORDER + 1); i++) {
        free_lists[i] = NULL;
    }

    // Loop through memory map entries
    for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];

        // Only process USABLE memory regions
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uintptr_t base = entry->base;
            uintptr_t length = entry->length;

            // Align base address and length
            uintptr_t aligned_base = (base + (1UL << MIN_ORDER) - 1) & ~((1UL << MIN_ORDER) - 1);
            uintptr_t aligned_length = length - (aligned_base - base);
            aligned_length &= ~((1UL << MIN_ORDER) - 1);

            uintptr_t addr = aligned_base;
            uintptr_t end = aligned_base + aligned_length;

            while (addr < end) {
                int order = MAX_ORDER;
                uintptr_t block_size = 1UL << order;

                // Find the largest block that fits and is aligned
                while (order >= MIN_ORDER) {
                    block_size = 1UL << order;
                    if (addr % block_size == 0 && (addr + block_size) <= end) {
                        break;
                    }
                    order--;
                }

                struct free_block *block = (struct free_block *)(addr + hhdm_offset);
                block->next = free_lists[order - MIN_ORDER];
                free_lists[order - MIN_ORDER] = block;

                addr += block_size;
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
    if (order > (MAX_ORDER - MIN_ORDER))
        return NULL;

    if (free_lists[order] == NULL) {
        struct free_block *buddy = split_block(order + 1);
        if (buddy == NULL)
            return NULL;

        size_t block_size = 1UL << (order + MIN_ORDER + 1);
        struct free_block *buddy1 = buddy;
        struct free_block *buddy2 = (struct free_block *)((uintptr_t)buddy + (block_size / 2));

        buddy1->next = NULL;
        buddy2->next = NULL;

        free_lists[order] = buddy2;
        return buddy1;
    }

    struct free_block *block = free_lists[order];
    free_lists[order] = block->next;
    return block;
}

void* buddy_alloc(size_t size) {
    spin_lock(&buddy_lock);
    
    int order = find_order(size + sizeof(struct alloc_header));
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
    
    // Thêm metadata trước vùng nhớ được cấp phát
    struct alloc_header *header = (struct alloc_header *)block;
    header->order = order;
    
    // Trả về địa chỉ bắt đầu của khối bộ nhớ được cấp phát (đã được canh chỉnh)
    void *user_ptr = (void *)block;
    
    spin_unlock(&buddy_lock);
    return user_ptr;
}

void buddy_free(void *ptr) {
    if (ptr == NULL)
        return;

    spin_lock(&buddy_lock);

    // Truy cập vào metadata trong khối bộ nhớ được cấp phát
    struct alloc_header *header = (struct alloc_header *)ptr; // Vì user_ptr = block
    int order = header->order;

    if (order < MIN_ORDER || order > MAX_ORDER) {
        // Invalid free
        spin_unlock(&buddy_lock);
        return;
    }

    uintptr_t block_addr = virt_to_phys((uintptr_t)header); // Chuyển đổi địa chỉ ảo sang vật lý

    while (order < (MAX_ORDER - MIN_ORDER)) {
        size_t block_size = 1UL << (order + MIN_ORDER);
        uintptr_t buddy_addr = block_addr ^ block_size;
        struct free_block *buddy = (struct free_block *)(buddy_addr + hhdm_offset);

        // Tìm buddy trong danh sách free
        struct free_block **current = &free_lists[order - MIN_ORDER];
        bool buddy_found = false;
        while (*current) {
            if (*current == buddy) {
                // Loại bỏ buddy khỏi danh sách free
                *current = buddy->next;
                buddy_found = true;
                break;
            }
            current = &(*current)->next;
        }

        if (!buddy_found)
            break;

        // Kết hợp buddies
        if (block_addr > buddy_addr)
            block_addr = buddy_addr;

        order++;
    }

    // Thêm khối đã kết hợp vào danh sách free
    struct free_block *block = (struct free_block *)(block_addr + hhdm_offset);
    block->next = free_lists[order - MIN_ORDER];
    free_lists[order - MIN_ORDER] = block;

    spin_unlock(&buddy_lock);
}

void buddy_print_free_memory() {
    spin_lock(&buddy_lock);

    size_t total_free_memory = 0;

    for (int order = 0; order <= MAX_ORDER - MIN_ORDER; order++) {
        size_t block_size = 1UL << (order + MIN_ORDER);  // Calculate block size for this order
        struct free_block *block = free_lists[order];
        size_t block_count = 0;

        // Traverse the free list for this order
        while (block) {
            block_count++;
            block = block->next;
        }

        total_free_memory += block_count * block_size;
    }

    spin_unlock(&buddy_lock);

    // Print the total free memory
    kprintf("Total free memory: %d bytes\n", total_free_memory);
}