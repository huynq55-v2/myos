#include "syscall_user.h"

long syscall(long number, long arg1, long arg2, long arg3) {
    long ret;
    // Assuming you use interrupt 0x80 for syscalls
    asm volatile (
        "int $0x80"
        : "=a" (ret)
        : "0" (number), "D" (arg1), "S" (arg2), "d" (arg3)
        : "rcx", "r11", "memory"
    );
    return ret;
}

// Syscall wrappers
ssize_t write(int fd, const void *buf, size_t count) {
    return syscall(SYSCALL_WRITE, fd, (long)buf, count);
}

ssize_t read(int fd, void *buf, size_t count) {
    return syscall(SYSCALL_READ, fd, (long)buf, count);
}

int close(int fd) {
    return syscall(SYSCALL_CLOSE, fd, 0, 0);
}

int fstat(int fd, struct stat *statbuf) {
    return syscall(SYSCALL_FSTAT, fd, (long)statbuf, 0);
}

int isatty(int fd) {
    return syscall(SYSCALL_ISATTY, fd, 0, 0);
}

off_t lseek(int fd, off_t offset, int whence) {
    return syscall(SYSCALL_LSEEK, fd, offset, whence);
}

void _exit(int status) {
    syscall(SYSCALL_EXIT, status, 0, 0);
    while(1) {} // Should never return
}

pid_t kill(pid_t pid, int sig) {
    return syscall(SYSCALL_KILL, pid, sig, 0);
}

pid_t getpid(void) {
    return syscall(SYSCALL_GETPID, 0, 0, 0);
}

void *sbrk(intptr_t increment) {
    // Giả sử không cần quản lý heap động, chỉ trả về lỗi
    return (void *)-1;
}