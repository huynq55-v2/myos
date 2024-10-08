// syscall_handler.c
#include "syscall_handler.h"
#include "syscall.h"
#include "klibc.h"
#include "graphics.h"
#include "process.h"
#include "memory_manager.h"

typedef int pid_t;
typedef long off_t;

/**
 * Implementation of the write syscall. Writes data from a buffer to a file
 * descriptor.
 *
 * @param fd The file descriptor to write to. Currently only fd=1 (standard
 * output) is supported.
 * @param buf The buffer to write from.
 * @param count The number of bytes to write from @p buf.
 *
 * @return The number of bytes written, or -1 on failure.
 */
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

// Implement the exit syscall
void syscall_exit(int status) {
    kprintf("Process exited with status %d\n", status);
    // Implement process termination logic here
    // process_exit(status);
}

extern uint64_t current_pid;
// Implement the getpid syscall
pid_t syscall_getpid() {
    return current_pid;
}

ssize_t syscall_read(int fd, void *buf, size_t count) {
    // Implement read functionality
    // For now, return -1 to indicate it's not implemented
    return -1;
}

ssize_t syscall_close(int fd) {
    // Implement close functionality
    return -1;
}

ssize_t syscall_fstat(int fd, struct stat *statbuf) {
    // Implement fstat functionality
    return -1;
}

ssize_t syscall_isatty(int fd) {
    // Implement isatty functionality
    return -1;
}

ssize_t syscall_lseek(int fd, off_t offset, int whence) {
    // Implement lseek functionality
    return -1;
}

ssize_t syscall_kill(pid_t pid, int sig) {
    // Implement kill functionality
    return -1;
}


/**
 * The syscall handler function. This function is called by the kernel whenever
 * a user process invokes a syscall. It takes the syscall number and up to 3
 * arguments, and dispatches to the correct syscall handler. If the syscall is
 * not supported, it prints an error message and returns -1. If the syscall is
 * supported, it calls the appropriate syscall handler and returns the result.
 *
 * @param syscall_number The syscall number to handle.
 * @param arg1 The first argument to the syscall.
 * @param arg2 The second argument to the syscall.
 * @param arg3 The third argument to the syscall.
 *
 * @return The result of the syscall, or -1 on error.
 */
ssize_t syscall_handler_c(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    ssize_t ret = -1; // Default return value for errors

    switch (syscall_number) {
        case SYSCALL_WRITE:
            ret = syscall_write((int)arg1, (const void *)arg2, (size_t)arg3);
            break;
        case SYSCALL_READ:
            ret = syscall_read((int)arg1, (void *)arg2, (size_t)arg3);
            break;
        case SYSCALL_CLOSE:
            ret = syscall_close((int)arg1);
            break;
        case SYSCALL_FSTAT:
            ret = syscall_fstat((int)arg1, (struct stat *)arg2);
            break;
        case SYSCALL_ISATTY:
            ret = syscall_isatty((int)arg1);
            break;
        case SYSCALL_LSEEK:
            ret = syscall_lseek((int)arg1, (off_t)arg2, (int)arg3);
            break;
        case SYSCALL_SBRK:
            // ret = syscall_sbrk((intptr_t)arg1);
            break;
        case SYSCALL_EXIT:
            syscall_exit((int)arg1);
            break;
        case SYSCALL_KILL:
            ret = syscall_kill((pid_t)arg1, (int)arg2);
            break;
        case SYSCALL_GETPID:
            ret = syscall_getpid();
            break;
        // Add more syscalls here
        default:
            kprintf("Syscall Handler: Unknown syscall number %llu\n", syscall_number);
            break;
    }

    return ret;
}