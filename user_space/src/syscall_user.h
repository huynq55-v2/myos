#ifndef SYSCALL_USER_H
#define SYSCALL_USER_H

#include <stdint.h>
#include <sys/types.h> // For ssize_t, off_t, pid_t
#include <sys/stat.h>  // For struct stat

// Define the syscall numbers (must match those in syscall.h)
#define SYSCALL_WRITE   1
#define SYSCALL_READ    2
#define SYSCALL_CLOSE   3
#define SYSCALL_FSTAT   4
#define SYSCALL_ISATTY  5
#define SYSCALL_LSEEK   6
#define SYSCALL_SBRK    7
#define SYSCALL_EXIT    8
#define SYSCALL_KILL    9
#define SYSCALL_GETPID  10

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

#endif // SYSCALL_USER_H
