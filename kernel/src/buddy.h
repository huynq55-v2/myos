#ifndef BUDDY_H
#define BUDDY_H

#include <stddef.h>
#include <stdint.h>
#include "limine.h" // Include limine.h for memory map structures

// Initialize the buddy allocator with the memory map response from Limine
void buddy_init(struct limine_memmap_response *memmap_response);

// Allocate memory of the given size (in bytes)
void *buddy_alloc(size_t size);

// Free the previously allocated memory pointed to by ptr
void buddy_free(void *ptr);

extern uintptr_t hhdm_offset;

#endif // BUDDY_H
