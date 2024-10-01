// process.h
#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} process_state_t;

typedef struct process {
    uint64_t pid;
    uint64_t entry_point;
    process_state_t state;
    struct process *next;
    // Additional fields can be added (e.g., registers, memory maps)
} process_t;

// Initialize process management
void process_init();

// Create a new process from an ELF binary
process_t* process_create(const uint8_t *elf_data, size_t size);

// Schedule the next process to run
void schedule();

// Get the current running process
process_t* current_process();

// Terminate the current process
void process_terminate();

#endif // PROCESS_H
