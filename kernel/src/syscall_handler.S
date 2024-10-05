.global syscall_handler
syscall_handler:
    cli                     // Tắt ngắt

    // Lưu các thanh ghi không phải volatile
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

    // Lưu các tham số syscall vào các thanh ghi tạm thời
    movq %rdi, %r10         // R10 = arg1 (fd)
    movq %rsi, %r11         // R11 = arg2 (buf)
    movq %rdx, %r12         // R12 = arg3 (count)

    // Di chuyển số syscall và các tham số vào thanh ghi theo quy ước
    movq %rax, %rdi          // RDI = số syscall (RAX)
    movq %r10, %rsi          // RSI = arg1 (fd)
    movq %r11, %rdx          // RDX = arg2 (buf)
    movq %r12, %rcx          // RCX = arg3 (count)

    // Căn chỉnh stack trước khi gọi hàm C
    subq $8, %rsp             // Căn chỉnh stack

    // Gọi hàm xử lý syscall trong C
    call syscall_handler_c    // Giá trị trả về được lưu trong RAX

    // Khôi phục căn chỉnh stack
    addq $8, %rsp

    // Khôi phục các thanh ghi không phải volatile
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

    sti                     // Bật ngắt
    iretq                   // Trả về từ interrupt
