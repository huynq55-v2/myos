// hello_user.c
#include "syscall_user.h"
#include <stdio.h>


void main() {
    pid_t pid = fork();

    if (pid < 0) {
        // Lỗi khi gọi fork
        printf("Fork failed\n");
        return 1;
    }

    if (pid == 0) {
        // Đây là tiến trình con
        printf("Hello from child process! PID: %ld\n", (long)getpid());
    } else {
        // Đây là tiến trình cha
        printf("Hello from parent process! Child PID: %ld\n", (long)pid);
    }

    return 0;
}
