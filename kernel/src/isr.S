.extern isr_handler_c

.macro ISR_NO_ERROR_CODE isr_num, vector_num
    .global isr\isr_num
isr\isr_num:
    cli
    pushq $0                # Đẩy mã lỗi giả (0) để giữ stack alignment
    pushq %rax
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

    movq $\vector_num, %rdi   # Tham số 1: số vector
    movq %rsp, %rsi           # Tham số 2: con trỏ stack
    call isr_handler_c

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
    popq %rax
    addq $8, %rsp             # Bỏ qua mã lỗi giả
    sti
    iretq
.endm

.macro ISR_WITH_ERROR_CODE isr_num, vector_num
    .global isr\isr_num
isr\isr_num:
    cli
    # Mã lỗi đã được CPU đẩy vào stack, không cần đẩy thêm
    pushq %rax
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

    movq $\vector_num, %rdi   # Tham số 1: số vector
    movq %rsp, %rsi           # Tham số 2: con trỏ stack
    call isr_handler_c

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
    popq %rax
    sti
    iretq
.endm

# Sử dụng macro phù hợp cho từng ngoại lệ
ISR_NO_ERROR_CODE 0, 0       # Chia cho 0, vector 0, không có mã lỗi
ISR_NO_ERROR_CODE 3, 3       # Breakpoint, vector 3, không có mã lỗi
