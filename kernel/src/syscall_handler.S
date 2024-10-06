// syscall_handler.asm
.global syscall_handler
syscall_handler:
    cli                     // Disable interrupts

    // Save non-volatile registers
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    // Move syscall arguments to temporary registers
    movq %rdi, %r10         // R10 = arg1
    movq %rsi, %r11         // R11 = arg2
    movq %rdx, %r12         // R12 = arg3

    // Prepare arguments for the C handler
    movq %rax, %rdi         // RDI = syscall_number
    movq %r10, %rsi         // RSI = arg1
    movq %r11, %rdx         // RDX = arg2
    movq %r12, %rcx         // RCX = arg3

    // Align the stack
    subq $8, %rsp

    // Call the C syscall handler
    call syscall_handler_c   // Return value in RAX

    // Restore stack alignment
    addq $8, %rsp

    // Restore non-volatile registers
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx

    sti                     // Enable interrupts
    iretq                   // Return from interrupt
