// start.c
#include "syscall_user.h"      // Định nghĩa syscall_exit nếu cần

// Khai báo hàm main
int main();

// Khai báo syscall _exit
void _exit(int status);

// Entry point của chương trình
void _start() {
    int status;

    // Gọi hàm main
    status = main();

    // Gọi syscall _exit với giá trị trả về từ main
    _exit(status);

    // Nếu syscall_exit không kết thúc tiến trình, dừng hệ thống
    while (1) {
        __asm__ volatile("hlt");
    }
}
