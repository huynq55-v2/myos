// hello_user.c
#include <stdio.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        // Lỗi khi gọi fork
        printf("Fork failed!\n");
    } else if (pid == 0) {
        // Đây là tiến trình con
        printf("Hello from child process!\n");
    } else {
        // Đây là tiến trình cha
        printf("Hello from parent process! Child PID: %d\n", pid);

        // Chờ đợi tiến trình con kết thúc
        int status;
        waitpid(pid, &status, 0);  // Chờ tiến trình con kết thúc
        printf("Child process %d exited with status %d\n", pid, status);
    }

    return 0;
}