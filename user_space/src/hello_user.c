// hello_user.c
#include "syscall_user.h"
#include <stdio.h>


void main() {
    const char *msg = "Hello, World from User Space!\n";
    while (1)
    {
        printf(msg);
    }
    
    printf(msg);
    while(1) {}
}
