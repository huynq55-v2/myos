.section .text
.global isr0
.global isr1
.global isr2
.global isr3
.global isr4
.global isr5
.global isr6
.global isr7
.global isr8
.global isr9
.global isr10
.global isr11
.global isr12
.global isr13
.global isr14
.global isr15
.global isr16
.global isr17
.global isr18
.global isr19
.global isr20
.global isr21
.global isr22
.global isr23
.global isr24
.global isr25
.global isr26
.global isr27
.global isr28
.global isr29
.global isr30
.global isr31
.global isr32

// Macro để định nghĩa ISR không có mã lỗi
.macro ISR_NO_ERROR_CODE isr_num, vector_num
    .global isr\isr_num
isr\isr_num:
    cli
    // Lưu trữ các thanh ghi vào stack
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
    // Lưu RFLAGS
    pushfq
    // Lưu mã đoạn (CS) và địa chỉ quay lại (RIP)
    movq $0x08, %rax       // Kernel Code Segment
    pushq %rax             // Lưu CS
    lea 1f(%rip), %rax     // Lấy địa chỉ của nhãn 1
    pushq %rax             // Lưu RIP
    // Gọi hàm xử lý ngắt
    movq $\vector_num, %rdi  // Sử dụng giá trị vector_num làm tham số đầu tiên
    movq %rsp, %rsi          // Tham số thứ hai: con trỏ đến stack frame
    call isr_handler_c
    // Phục hồi các thanh ghi
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
    addq $16, %rsp           // Bỏ qua CS và RIP
    popfq
    sti
    iretq
1:
.endm

// Macro để định nghĩa ISR có mã lỗi
.macro ISR_WITH_ERROR_CODE isr_num, vector_num
    .global isr\isr_num
isr\isr_num:
    cli
    // Lưu trữ các thanh ghi vào stack
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
    // Lưu RFLAGS
    pushfq
    // Lưu mã đoạn (CS) và địa chỉ quay lại (RIP)
    movq $0x08, %rax       // Kernel Code Segment
    pushq %rax             // Lưu CS
    lea 1f(%rip), %rax     // Lấy địa chỉ của nhãn 1
    pushq %rax             // Lưu RIP
    // Gọi hàm xử lý ngắt với mã lỗi
    movq $\vector_num, %rdi  // Sử dụng giá trị vector_num làm tham số đầu tiên
    movq %rsp, %rsi          // Tham số thứ hai: con trỏ đến stack frame (bao gồm error_code)
    call isr_handler_c
    // Phục hồi các thanh ghi
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
    addq $16, %rsp           // Bỏ qua CS và RIP
    popfq
    sti
    iretq
1:
.endm

// Định nghĩa các ISR từ 0 đến 31
ISR_NO_ERROR_CODE 0, 0     # Divide by Zero Exception
ISR_NO_ERROR_CODE 1, 1     # Debug Exception
ISR_NO_ERROR_CODE 2, 2     # Non-Maskable Interrupt
ISR_NO_ERROR_CODE 3, 3     # Breakpoint Exception
ISR_NO_ERROR_CODE 4, 4     # Overflow Exception
ISR_NO_ERROR_CODE 5, 5     # Bound Range Exceeded
ISR_NO_ERROR_CODE 6, 6     # Invalid Opcode
ISR_NO_ERROR_CODE 7, 7     # Device Not Available
ISR_WITH_ERROR_CODE 8, 8   # Double Fault
ISR_WITH_ERROR_CODE 9, 9   # Coprocessor Segment Overrun
ISR_WITH_ERROR_CODE 10, 10 # Invalid TSS
ISR_WITH_ERROR_CODE 11, 11 # Segment Not Present
ISR_WITH_ERROR_CODE 12, 12 # Stack-Segment Fault
ISR_WITH_ERROR_CODE 13, 13 # General Protection Fault
ISR_WITH_ERROR_CODE 14, 14 # Page Fault
ISR_NO_ERROR_CODE 15, 15   # Reserved
ISR_NO_ERROR_CODE 16, 16   # x87 Floating-Point Exception
ISR_NO_ERROR_CODE 17, 17   # Alignment Check
ISR_WITH_ERROR_CODE 18, 18 # Machine Check
ISR_NO_ERROR_CODE 19, 19   # SIMD Floating-Point Exception
ISR_NO_ERROR_CODE 20, 20   # Virtualization Exception
ISR_NO_ERROR_CODE 21, 21   # Reserved
ISR_NO_ERROR_CODE 22, 22   # Reserved
ISR_NO_ERROR_CODE 23, 23   # Reserved
ISR_NO_ERROR_CODE 24, 24   # Reserved
ISR_NO_ERROR_CODE 25, 25   # Reserved
ISR_NO_ERROR_CODE 26, 26   # Reserved
ISR_NO_ERROR_CODE 27, 27   # Reserved
ISR_NO_ERROR_CODE 28, 28   # Reserved
ISR_NO_ERROR_CODE 29, 29   # Reserved
ISR_WITH_ERROR_CODE 30, 30 # Security Exception
ISR_NO_ERROR_CODE 31, 31   # Reserved
ISR_NO_ERROR_CODE 32, 32   # APIC Timer Interrupt
