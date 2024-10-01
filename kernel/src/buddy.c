#include "buddy.h"
#include "klibc.h"

// Constants
#define MAX_ORDER 10      // Maximum order (corresponds to block sizes up to 4MB for 4KB pages)
#define PAGE_SIZE 4096    // Page size in bytes

// Buddy system data structures
struct buddy {
    uint32_t *bitmap[MAX_ORDER + 1]; // Bitmaps for each order
    size_t bitmap_size[MAX_ORDER + 1];
    uintptr_t base_addr;             // Base address of the managed memory region
    size_t total_pages;              // Total number of pages in the managed region
};

// Static instance of the buddy allocator
static struct buddy buddy_allocator;

extern volatile struct limine_hhdm_request hhdm_request;

uintptr_t hhdm_offset;

// Initialize the buddy allocator with the USABLE memory regions
void buddy_init(struct limine_memmap_response *memmap_response) {
    
    hhdm_offset = hhdm_request.response->offset;

    // Find the largest contiguous USABLE memory region
    uintptr_t max_base = 0;
    size_t max_length = 0;

    for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_response->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            if (entry->length > max_length) {
                max_length = entry->length;
                max_base = entry->base;
            }
        }
    }

    if (max_length == 0) {
        // No USABLE memory found
        return;
    }

    // Initialize the buddy allocator with the largest USABLE region
    buddy_allocator.base_addr = max_base;
    buddy_allocator.total_pages = max_length / PAGE_SIZE;

    // Initialize bitmaps for each order
    for (int order = 0; order <= MAX_ORDER; order++) {
        size_t num_blocks = buddy_allocator.total_pages >> order;
        size_t bitmap_bytes = (num_blocks + 31) / 32 * sizeof(uint32_t); // Round up to 32 bits
        buddy_allocator.bitmap_size[order] = bitmap_bytes;
        buddy_allocator.bitmap[order] = (uint32_t *)0; // Will allocate memory for bitmaps later
    }

    // Allocate memory for bitmaps from the start of the memory region
    uintptr_t bitmap_addr = buddy_allocator.base_addr;
    size_t bitmap_total_size = 0;
    for (int order = 0; order <= MAX_ORDER; order++) {
        bitmap_total_size += buddy_allocator.bitmap_size[order];
    }

    if (bitmap_total_size > max_length) {
        // Not enough memory for bitmaps
        return;
    }

    // Assign memory to bitmaps and initialize them
    uintptr_t current_addr = bitmap_addr;
    for (int order = 0; order <= MAX_ORDER; order++) {
        buddy_allocator.bitmap[order] = (uint32_t *)(current_addr + hhdm_offset);
        memset(buddy_allocator.bitmap[order], 0, buddy_allocator.bitmap_size[order]);
        current_addr += buddy_allocator.bitmap_size[order];
    }

    // Update base address and total pages to exclude bitmap memory
    buddy_allocator.base_addr += bitmap_total_size;
    buddy_allocator.total_pages -= bitmap_total_size / PAGE_SIZE;

    // Mark all memory as free in the highest possible order
    int highest_order = MAX_ORDER;
    size_t num_blocks = buddy_allocator.total_pages >> highest_order;
    while (num_blocks == 0 && highest_order > 0) {
        highest_order--;
        num_blocks = buddy_allocator.total_pages >> highest_order;
    }

    // Set the highest order bitmap to free
    for (size_t i = 0; i < num_blocks; i++) {
        size_t idx = i / 32;
        size_t bit = i % 32;
        buddy_allocator.bitmap[highest_order][idx] |= (1 << bit);
    }

    // Lower order bitmaps are initially empty (all blocks are in the highest order)
}

// Allocate memory of the given size (in bytes)
void *buddy_alloc(size_t size) {
    // Calculate the number of pages needed
    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    // Find the smallest order that can fit the requested size
    int order;
    for (order = 0; order <= MAX_ORDER; order++) {
        if ((1UL << order) >= pages)
            break;
    }

    if (order > MAX_ORDER) {
        // Requested size is too big
        return NULL;
    }

    // Search for a free block in this order or higher
    int current_order = order;
    while (current_order <= MAX_ORDER) {
        size_t num_blocks = buddy_allocator.total_pages >> current_order;
        uint32_t *bitmap = buddy_allocator.bitmap[current_order];

        for (size_t i = 0; i < num_blocks; i++) {
            size_t idx = i / 32;
            size_t bit = i % 32;

            if (bitmap[idx] & (1 << bit)) {
                // Found a free block
                // Mark it as used
                bitmap[idx] &= ~(1 << bit);

                // Split the block into lower orders if necessary
                while (current_order > order) {
                    current_order--;
                    // Mark the buddy block as free in the lower order
                    size_t buddy_block = i * 2 + 1;
                    size_t buddy_idx = buddy_block / 32;
                    size_t buddy_bit = buddy_block % 32;
                    buddy_allocator.bitmap[current_order][buddy_idx] |= (1 << buddy_bit);
                    i *= 2; // Continue with the first half
                }

                // Return the address of the allocated block
                uintptr_t addr = buddy_allocator.base_addr + (i << (current_order + 12)); // 12 bits for page size
                return (void *)addr;
            }
        }
        current_order++;
    }

    // No suitable block found
    return NULL;
}

// Free the previously allocated memory pointed to by ptr
void buddy_free(void *ptr) {
    uintptr_t addr = (uintptr_t)ptr;

    if (addr < buddy_allocator.base_addr || addr >= buddy_allocator.base_addr + (buddy_allocator.total_pages * PAGE_SIZE)) {
        // Address out of range
        return;
    }

    // Calculate the page index
    size_t offset = addr - buddy_allocator.base_addr;
    size_t page_idx = offset / PAGE_SIZE;

    int order = 0; // Start from the smallest order

    // Coalesce with buddy blocks if possible
    while (order <= MAX_ORDER) {
        size_t block_idx = page_idx >> order;
        size_t idx = block_idx / 32;
        size_t bit = block_idx % 32;

        // Check if the buddy block is free
        size_t buddy_block_idx = block_idx ^ 1; // Toggle the last bit to find the buddy
        size_t buddy_idx = buddy_block_idx / 32;
        size_t buddy_bit = buddy_block_idx % 32;

        uint32_t *bitmap = buddy_allocator.bitmap[order];

        if (bitmap[buddy_idx] & (1 << buddy_bit)) {
            // Buddy block is free, coalesce
            // Mark buddy block as used
            bitmap[buddy_idx] &= ~(1 << buddy_bit);
            // Move up to the next order
            page_idx &= ~(1UL << order);
            order++;
        } else {
            // Buddy block is not free
            // Mark current block as free
            bitmap[idx] |= (1 << bit);
            break;
        }
    }
}
