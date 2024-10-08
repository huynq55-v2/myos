// context_switcher.S
.global _save_context
.global _switch_to_context
.global switch_to_user_space

.section .text

// Hàm lưu ngữ cảnh
_save_context:
    // Argument: rdi = pointer to cpu_context_t

    // Lưu các thanh ghi đa dụng vào cấu trúc cpu_context_t
    movq %rax, 0x00(%rdi)    // rax
    movq %rbx, 0x08(%rdi)    // rbx
    movq %rcx, 0x10(%rdi)    // rcx
    movq %rdx, 0x18(%rdi)    // rdx
    movq %rsi, 0x20(%rdi)    // rsi
    movq %rsp, 0x30(%rdi)    // rsp
    movq %rbp, 0x38(%rdi)    // rbp
    movq %r8,  0x40(%rdi)    // r8
    movq %r9,  0x48(%rdi)    // r9
    movq %r10, 0x50(%rdi)    // r10
    movq %r11, 0x58(%rdi)    // r11
    movq %r12, 0x60(%rdi)    // r12
    movq %r13, 0x68(%rdi)    // r13
    movq %r14, 0x70(%rdi)    // r14
    movq %r15, 0x78(%rdi)    // r15

    // Lưu RFLAGS vào cpu_context_t
    pushfq
    popq %rax
    movq %rax, 0x88(%rdi)    // rflags

    // Lưu RIP vào cpu_context_t
    lea 1f, %rax
    movq %rax, 0x80(%rdi)    // rip

    movq %rdi, 0x28(%rdi)    // rdi

    ret
1:

// Hàm chuyển đổi ngữ cảnh
_switch_to_context:
    // Argument: rdi = pointer to cpu_context_t

    // Tải CR3 từ cpu_context_t và cập nhật CR3
    movq 0x90(%rdi), %rax    // cr3 tại offset 0x90
    movq %rax, %cr3

    // Tải các thanh ghi đa dụng từ cpu_context_t (except rdi and rsp)
    movq 0x00(%rdi), %rax    // rax
    movq 0x08(%rdi), %rbx    // rbx
    movq 0x10(%rdi), %rcx    // rcx
    movq 0x18(%rdi), %rdx    // rdx
    movq 0x20(%rdi), %rsi    // rsi
    // Do not restore %rdi yet
    // Do not restore %rsp yet
    movq 0x38(%rdi), %rbp    // rbp
    movq 0x40(%rdi), %r8     // r8
    movq 0x48(%rdi), %r9     // r9
    movq 0x50(%rdi), %r10    // r10
    movq 0x58(%rdi), %r11    // r11
    movq 0x60(%rdi), %r12    // r12
    movq 0x68(%rdi), %r13    // r13
    movq 0x70(%rdi), %r14    // r14
    movq 0x78(%rdi), %r15    // r15

    // Thiết lập stack frame cho iretq
    // Push SS
    pushq $0x23              // Segment selector cho user data segment (thường là 0x23)
    // Push RSP (user mode stack pointer)
    movq 0x30(%rdi), %rax    // Lấy giá trị %rsp từ cpu_context_t
    pushq %rax
    // Push RFLAGS
    movq 0x88(%rdi), %rax    // rflags tại offset 0x88
    pushq %rax
    // Push CS
    pushq $0x1B              // Segment selector cho user code segment (thường là 0x1B)
    // Push RIP
    movq 0x80(%rdi), %rax    // rip tại offset 0x80
    pushq %rax               // Đẩy rip lên stack

    // Bây giờ khôi phục %rdi
    movq 0x28(%rdi), %rdi    // rdi

    // Không cần khôi phục %rsp vì iretq sẽ xử lý

    // Thực hiện chuyển đổi ngữ cảnh bằng iretq
    iretq

// Hàm chuyển đổi sang user space
switch_to_user_space:
    // Argument: rdi = pointer to new cpu_context_t
    call _switch_to_context
