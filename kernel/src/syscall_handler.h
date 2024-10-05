// syscall_handler.h
#ifndef SYSCALL_HANDLER_H
#define SYSCALL_HANDLER_H

#include <stdint.h>
#include <stddef.h>

typedef long ssize_t;

// Syscall handler function
ssize_t syscall_handler_c(uint64_t syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3);

// Declare the write syscall function
ssize_t syscall_write(int fd, const void *buf, size_t count);

#endif // SYSCALL_HANDLER_H
