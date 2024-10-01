// process.c
#include "process.h"
#include "elf_loader.h"
#include "buddy.h"
#include "klibc.h"
#include "graphics.h"

static process_t *process_list = NULL;
static process_t *current = NULL;
static uint64_t next_pid = 1;

void process_init() {
    // Initialize the process list
    process_list = NULL;
    current = NULL;
    next_pid = 1;
}

process_t* process_create(const uint8_t *elf_data, size_t size) {
    process_t *proc = buddy_alloc(sizeof(process_t));
    if (!proc) {
        kprintf("Process: Failed to allocate process structure\n");
        return NULL;
    }

    memset(proc, 0, sizeof(process_t));
    proc->pid = next_pid++;
    proc->state = PROCESS_READY;
    proc->next = NULL;

    // Load ELF binary into process's address space
    if (!load_elf(elf_data, size, proc)) {
        kprintf("Process: Failed to load ELF binary\n");
        buddy_free(proc);
        return NULL;
    }

    // Add to the process list
    if (!process_list) {
        process_list = proc;
    } else {
        process_t *iter = process_list;
        while (iter->next)
            iter = iter->next;
        iter->next = proc;
    }

    kprintf("Process: Created process PID %d\n", proc->pid);
    return proc;
}

process_t* current_process() {
    return current;
}

void schedule() {
    if (!process_list) {
        kprintf("Scheduler: No processes to schedule\n");
        return;
    }

    // Simple round-robin scheduler
    if (!current) {
        current = process_list;
    } else {
        current->state = PROCESS_READY;
        current = current->next ? current->next : process_list;
    }

    current->state = PROCESS_RUNNING;
    // Switch context to current process
    // This would involve setting up the CPU registers, switching stacks, etc.
    // For simplicity, we'll assume a function switch_to_process exists
    switch_to_process(current);
}

void process_terminate() {
    if (!current) {
        kprintf("Process: No current process to terminate\n");
        return;
    }

    kprintf("Process: Terminating PID %d\n", current->pid);
    current->state = PROCESS_TERMINATED;

    // Remove from the process list
    if (process_list == current) {
        process_list = current->next;
    } else {
        process_t *iter = process_list;
        while (iter->next && iter->next != current)
            iter = iter->next;
        if (iter->next)
            iter->next = current->next;
    }

    // Free process resources
    buddy_free(current);
    current = NULL;

    // Schedule next process
    schedule();
}
