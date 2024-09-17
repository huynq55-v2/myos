// isr.S

.extern isr_handler_c

.macro ISR_NO_ERROR_CODE isr_num, vector_num
    .global isr\isr_num
isr\isr_num:
    cli
    andq $~0xF, %rsp   # Căn chỉnh stack 16 byte

    # Đẩy một mã lỗi giả lên stack để đồng nhất với ISR có mã lỗi
    pushq $0

    # Lưu các thanh ghi
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

    # Khôi phục các thanh ghi
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

    # Loại bỏ mã lỗi giả khỏi stack
    addq $8, %rsp

    sti
    iretq
.endm

.macro ISR_WITH_ERROR_CODE isr_num, vector_num
    .global isr\isr_num
isr\isr_num:
    cli
    andq $~0xF, %rsp   # Căn chỉnh stack 16 byte

    # Stack tại thời điểm này:
    # [Error Code]
    # [Return RIP]
    # [Return CS]
    # [RFLAGS]

    # Giữ nguyên Error Code trên stack

    # Lưu các thanh ghi
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

    # Khôi phục các thanh ghi
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

    # Loại bỏ Error Code khỏi stack
    addq $8, %rsp

    sti
    iretq
.endm

# Sử dụng macro cho các ISR
ISR_NO_ERROR_CODE 0, 0       # Division by zero
ISR_NO_ERROR_CODE 3, 3       # Breakpoint
ISR_WITH_ERROR_CODE 8, 8     # Double fault (có mã lỗi)
