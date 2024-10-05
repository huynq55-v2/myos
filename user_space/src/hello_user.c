// hello_user.c
#include "syscall_user.h"
#include <stdio.h>

void _start() {
    const char *msg = "Hello, World from User Space!\n";
    write(1, msg, 28); // fd = 1 (stdout), length = 28
    // exit(5);

    // Exit the program (you might need to implement an exit syscall)
    // For simplicity, loop indefinitely
    printf("This is printf in user space!\n");
    while (1) {}
}

void main() {
    _start();
}