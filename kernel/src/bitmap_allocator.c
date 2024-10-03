#include "bitmap_allocator.h"

// Initializes the allocator
void bitmap_allocator_init(bitmap_allocator_t *allocator, uint8_t *bitmap_memory, uint64_t total_memory_size) {
    allocator->bitmap = bitmap_memory;
    allocator->size = total_memory_size * 8; // Number of bits
    allocator->total_blocks = allocator->size; // Each bit represents one block

    // Initialize bitmap to 0 (all blocks free)
    for (uint64_t i = 0; i < total_memory_size; i++) {
        allocator->bitmap[i] = 0;
    }
}


/**
 * Allocates a single block. Returns block index or (uint64_t)-1 on failure.
 *
 * The function iterates over the bitmap and finds the first free block.
 * If no free block is found, it returns (uint64_t)-1.
 *
 * @param allocator The allocator from which to allocate the block.
 *
 * @returns The index of the allocated block or (uint64_t)-1 on failure.
 */
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

/**
 * Frees a single block from the allocator.
 *
 * @param allocator The allocator from which to free the block.
 * @param block_index The index of the block to free.
 *
 * If `block_index` is invalid (i.e. greater than or equal to
 * `allocator->total_blocks`), the function does nothing.
 *
 * Otherwise, the function simply clears the corresponding bit in the bitmap,
 * thus marking the block as free.
 */
void bitmap_free(bitmap_allocator_t *allocator, uint64_t block_index) {
    if (block_index >= allocator->total_blocks) {
        return; // Invalid block index
    }

    uint64_t byte = block_index / 8;
    uint8_t bit = block_index % 8;
    allocator->bitmap[byte] &= ~(1 << bit);
}


/**
 * Checks if a block is allocated in the allocator.
 *
 * The function takes a `bitmap_allocator_t *` and a `uint64_t block_index`
 * as parameters. It returns 1 if the block is allocated, 0 if it is free.
 *
 * If `block_index` is invalid (i.e. greater than or equal to
 * `allocator->total_blocks`), the function returns 0.
 *
 * Otherwise, the function checks the corresponding bit in the bitmap and
 * returns 1 if the bit is set, 0 if it is not.
 */
int bitmap_is_allocated(bitmap_allocator_t *allocator, uint64_t block_index) {
    if (block_index >= allocator->total_blocks) {
        return 0;
    }

    uint64_t byte = block_index / 8;
    uint8_t bit = block_index % 8;
    return (allocator->bitmap[byte] & (1 << bit)) != 0;
}


/**
 * Allocates `count` contiguous blocks from the allocator.
 *
 * The function takes a `bitmap_allocator_t *` and a `uint64_t count`
 * as parameters. It returns the index of the first block in the contiguous
 * range if successful, `(uint64_t)-1` otherwise.
 *
 * If `count` is invalid (i.e. 0 or greater than `allocator->total_blocks`),
 * the function returns `(uint64_t)-1`.
 *
 * Otherwise, the function checks the bitmap for `count` contiguous free
 * blocks and allocates them all if found. If no such range is found, the
 * function returns `(uint64_t)-1`.
 *
 * The function marks all allocated blocks as allocated in the bitmap.
 */
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


/**
 * Frees `count` contiguous blocks starting from `start_block` in the allocator.
 *
 * The function takes a `bitmap_allocator_t *` and a `uint64_t start_block`
 * and a `uint64_t count` as parameters. It marks all blocks in the range as
 * free in the bitmap.
 *
 * If `count` is invalid (i.e. 0 or greater than `allocator->total_blocks`) or
 * `start_block + count` is greater than `allocator->total_blocks`, the
 * function does nothing.
 *
 * Otherwise, the function marks all blocks in the range as free in the bitmap.
 */
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
