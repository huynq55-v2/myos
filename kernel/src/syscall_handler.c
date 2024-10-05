// syscall_handler.c
#include "syscall_handler.h"
#include "syscall.h"
#include "klibc.h"
#include "graphics.h"
#include "process.h"

// Implement the write syscall
ssize_t syscall_write(int fd, const void *buf, size_t count) {
    if (fd == 1) { // Standard output
        const char *str = (const char *)buf;
        for (size_t i = 0; i < count; i++) {
            put_char(str[i]);
        }
        return count;
    }

    // Handle other file descriptors if needed
    return -1; // Unsupported file descriptor
}

// Syscall handler implementation
ssize_t syscall_handler_c(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    ssize_t ret = -1; // Default return value for errors

    switch (syscall_number) {
        case SYSCALL_WRITE:
            ret = syscall_write((int)arg1, (const void *)arg2, (size_t)arg3);
            break;
        // Add more syscalls here
        default:
            kprintf("Syscall Handler: Unknown syscall number %llu\n", syscall_number);
            break;
    }

    return ret;
}
