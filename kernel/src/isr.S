.macro ISR_NO_ERROR_CODE isr_num, vector_num
    .global isr\isr_num
isr\isr_num:
    cli
    # Align stack to 16 bytes if needed
    andq $~0xF, %rsp
    # Set up parameters
    movq $\vector_num, %rdi       # First argument: vector_number
    movq %rsp, %rsi               # Second argument: pointer to stack frame
    # Call the common handler
    call isr_handler_c
    sti
    iretq
.endm

.macro ISR_WITH_ERROR_CODE isr_num, vector_num
    .global isr\isr_num
isr\isr_num:
    cli
    # Align stack to 16 bytes if needed
    andq $~0xF, %rsp
    # Set up parameters
    movq $\vector_num, %rdi       # First argument: vector_number
    movq %rsp, %rsi               # Second argument: pointer to stack frame (includes error code)
    # Call the common handler
    call isr_handler_c
    sti
    iretq
.endm

# Sử dụng các macro để định nghĩa ISR từ 0 đến 31
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
