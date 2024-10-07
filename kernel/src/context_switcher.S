// context_switcher.S
.global _save_context
.global _switch_to_context
.global switch_to_user_space

.section .text

// Hàm lưu ngữ cảnh
_save_context:
    // Argument: rdi = pointer to cpu_context_t
    // Lưu các thanh ghi vào cấu trúc context
    movq %rax, 0x00(%rdi)
    movq %rbx, 0x08(%rdi)
    movq %rcx, 0x10(%rdi)
    movq %rdx, 0x18(%rdi)
    movq %rsi, 0x20(%rdi)
    movq %rdi, 0x28(%rdi)
    movq %rbp, 0x30(%rdi)
    movq %r8,  0x38(%rdi)
    movq %r9,  0x40(%rdi)
    movq %r10, 0x48(%rdi)
    movq %r11, 0x50(%rdi)
    movq %r12, 0x58(%rdi)
    movq %r13, 0x60(%rdi)
    movq %r14, 0x68(%rdi)
    movq %r15, 0x70(%rdi)
    // Lưu RFLAGS
    pushfq
    popq %rax
    movq %rax, 0x78(%rdi)
    // Lưu RIP
    lea 1f, %rax
    movq %rax, 0x80(%rdi)
    ret
1:

// Hàm chuyển đổi ngữ cảnh
_switch_to_context:
    // Argument: rdi = pointer to new cpu_context_t
    // Lấy địa chỉ của cấu trúc ngữ cảnh mới
    movq 0x00(%rdi), %rax
    movq 0x08(%rdi), %rbx
    movq 0x10(%rdi), %rcx
    movq 0x18(%rdi), %rdx
    movq 0x20(%rdi), %rsi
    movq 0x28(%rdi), %rdi
    movq 0x30(%rdi), %rbp
    movq 0x38(%rdi), %r8
    movq 0x40(%rdi), %r9
    movq 0x48(%rdi), %r10
    movq 0x50(%rdi), %r11
    movq 0x58(%rdi), %r12
    movq 0x60(%rdi), %r13
    movq 0x68(%rdi), %r14
    movq 0x70(%rdi), %r15
    // Lấy RFLAGS
    movq 0x78(%rdi), %rax
    pushq %rax
    popfq
    // Lấy RIP
    movq 0x80(%rdi), %rax
    jmp *%rax

// Hàm chuyển đổi sang user space
switch_to_user_space:
    // Argument: rdi = pointer to new cpu_context_t
    call _switch_to_context
