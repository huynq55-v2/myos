#include "bitmap_allocator.h"

// Initializes the allocator
void bitmap_allocator_init(bitmap_allocator_t *allocator, uint8_t *bitmap_memory, uint64_t total_memory_size) {
    allocator->bitmap = bitmap_memory;
    allocator->size = total_memory_size * 8; // Number of bits
    allocator->total_blocks = allocator->size / BLOCK_SIZE;

    // Initialize bitmap to 0 (all blocks free)
    for (uint64_t i = 0; i < total_memory_size; i++) {
        allocator->bitmap[i] = 0;
    }
}

// Allocates a single block
uint64_t bitmap_alloc(bitmap_allocator_t *allocator) {
    for (uint64_t byte = 0; byte < allocator->size / 8; byte++) {
        if (allocator->bitmap[byte] != 0xFF) { // Not all bits are set
            for (uint8_t bit = 0; bit < 8; bit++) {
                if (!(allocator->bitmap[byte] & (1 << bit))) {
                    allocator->bitmap[byte] |= (1 << bit);
                    uint64_t block_index = byte * 8 + bit;
                    if (block_index < allocator->total_blocks) {
                        return block_index;
                    }
                    return (uint64_t)-1; // Out of range
                }
            }
        }
    }
    return (uint64_t)-1; // No free blocks
}

// Frees a single block
void bitmap_free(bitmap_allocator_t *allocator, uint64_t block_index) {
    uint64_t byte = block_index / 8;
    uint8_t bit = block_index % 8;
    if (byte < allocator->size / 8) {
        allocator->bitmap[byte] &= ~(1 << bit);
    }
}

// Checks if a block is allocated
int bitmap_is_allocated(bitmap_allocator_t *allocator, uint64_t block_index) {
    uint64_t byte = block_index / 8;
    uint8_t bit = block_index % 8;
    if (byte < allocator->size / 8) {
        return (allocator->bitmap[byte] & (1 << bit)) != 0;
    }
    return 0;
}

// Allocates 'count' contiguous blocks
uint64_t bitmap_alloc_contiguous(bitmap_allocator_t *allocator, uint64_t count) {
    if (count == 0 || count > allocator->total_blocks) {
        return (uint64_t)-1;
    }

    uint64_t consecutive = 0;
    uint64_t start_block = 0;

    for (uint64_t block = 0; block < allocator->total_blocks; block++) {
        if (!bitmap_is_allocated(allocator, block)) {
            if (consecutive == 0) {
                start_block = block;
            }
            consecutive++;
            if (consecutive == count) {
                // Mark all blocks as allocated
                for (uint64_t i = start_block; i < start_block + count; i++) {
                    uint64_t byte = i / 8;
                    uint8_t bit = i % 8;
                    allocator->bitmap[byte] |= (1 << bit);
                }
                return start_block;
            }
        } else {
            consecutive = 0;
        }
    }

    return (uint64_t)-1; // Not enough contiguous blocks found
}

// Frees 'count' contiguous blocks starting from 'start_block'
void bitmap_free_contiguous(bitmap_allocator_t *allocator, uint64_t start_block, uint64_t count) {
    if (count == 0 || (start_block + count) > allocator->total_blocks) {
        return; // Invalid parameters
    }

    for (uint64_t i = start_block; i < start_block + count; i++) {
        uint64_t byte = i / 8;
        uint8_t bit = i % 8;
        allocator->bitmap[byte] &= ~(1 << bit);
    }
}
