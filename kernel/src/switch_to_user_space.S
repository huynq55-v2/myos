// switch_to_user_space.S

.global switch_to_user_space_asm

switch_to_user_space_asm:
    // Thiết lập ngăn xếp
    movq %rdi, %rsp           // stack_pointer vào %rdi
    movq %rsi, %rbp           // Thiết lập rbp từ stack_pointer
    
    // Đẩy các giá trị lên ngăn xếp
    pushq $0x23               // SS: User Data Segment (selector 0x23)
    pushq %rsp                // RSP: Giá trị của ngăn xếp
    pushfq                    // RFLAGS
    pushq $0x1B               // CS: User Code Segment (selector 0x1B)
    pushq %rdx                // RIP: entry point (được truyền vào %rdx)

    // Chuyển sang user space
    iretq
