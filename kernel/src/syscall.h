// syscall.h
#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

// Syscall numbers
typedef enum {
    SYSCALL_WRITE = 1,
    SYSCALL_READ,
    SYSCALL_CLOSE,
    SYSCALL_FSTAT,
    SYSCALL_ISATTY,
    SYSCALL_LSEEK,
    SYSCALL_SBRK,
    SYSCALL_EXIT,
    SYSCALL_KILL,
    SYSCALL_GETPID,
    // Add more syscalls here as needed
} syscall_number_t;

#endif // SYSCALL_H
