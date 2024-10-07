#ifndef SYSCALL_USER_H
#define SYSCALL_USER_H

#include <stdint.h>
#include <sys/types.h> // For ssize_t, off_t, pid_t
#include <sys/stat.h>  // For struct stat

// Define the syscall numbers (must match those in syscall.h)
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
    SYSCALL_FORK,
    // Add more syscalls here as needed
} syscall_number_t;

// Generic syscall function
long syscall(long number, long arg1, long arg2, long arg3);

// Syscall wrappers
ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);
int close(int fd);
int fstat(int fd, struct stat *statbuf);
int isatty(int fd);
off_t lseek(int fd, off_t offset, int whence);
void _exit(int status);
pid_t kill(pid_t pid, int sig);
pid_t getpid(void);
void *sbrk(intptr_t increment);
pid_t fork(void);

#endif // SYSCALL_USER_H
