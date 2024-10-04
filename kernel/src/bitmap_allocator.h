#ifndef BITMAP_ALLOCATOR_H
#define BITMAP_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

// Bitmap Allocator structure
typedef struct {
    uint8_t *bitmap;        // Pointer to the bitmap
    uint64_t total_blocks;  // Total number of blocks managed
} bitmap_allocator_t;

// Initializes the allocator
void bitmap_allocator_init(bitmap_allocator_t *allocator, uint8_t *bitmap_memory, uint64_t total_memory_size);

// Allocates a single block. Returns block index or (uint64_t)-1 on failure.
uint64_t bitmap_alloc(bitmap_allocator_t *allocator);

// Frees a single block.
void bitmap_free(bitmap_allocator_t *allocator, uint64_t block_index);

// Checks if a block is allocated. Returns 1 if allocated, 0 if free.
int bitmap_is_allocated(bitmap_allocator_t *allocator, uint64_t block_index);

// Allocates 'count' contiguous blocks. Returns starting block index or (uint64_t)-1 on failure.
uint64_t bitmap_alloc_contiguous(bitmap_allocator_t *allocator, uint64_t count);

// Frees 'count' contiguous blocks starting from 'start_block'.
void bitmap_free_contiguous(bitmap_allocator_t *allocator, uint64_t start_block, uint64_t count);

#endif // BITMAP_ALLOCATOR_H
