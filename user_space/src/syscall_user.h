#ifndef SYSCALL_USER_H
#define SYSCALL_USER_H

#include <stdint.h>
#include <stddef.h>

typedef long ssize_t;

// Syscall numbers (must match kernel's syscall.h)
enum {
    SYSCALL_WRITE = 1,
    SYSCALL_EXIT = 2, // New syscall number for exit
    // Add more syscalls if needed
};

// Syscall wrapper for write
ssize_t write(int fd, const void *buf, size_t count) {
    ssize_t ret;
    asm volatile (
        "int $0x80;"         // Gọi syscall
        : "=a" (ret)          // Giá trị trả về
        : "a" ((uint64_t)SYSCALL_WRITE), // RAX = số syscall
          "D" ((uint64_t)fd),            // RDI = fd
          "S" ((uint64_t)buf),           // RSI = buf
          "d" ((uint64_t)count)          // RDX = count
        : "rcx", "r11", "memory"         // Các thanh ghi bị thay đổi
    );
    return ret;
}

// Syscall wrapper for exit
void exit(int status) {
    asm volatile (
        "int $0x80;"         // Trigger syscall
        :
        : "a" ((uint64_t)SYSCALL_EXIT), // RAX = syscall number
          "D" ((uint64_t)status)        // RDI = status
        : "rcx", "r11", "memory"         // Clobbered registers
    );
}

#endif // SYSCALL_USER_H
