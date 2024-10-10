# File: context_switch.S

        .global _save_context
_save_context:
        # Input: %rdi = pointer to cpu_context_t

        # Lưu các thanh ghi tổng quát
        movq %rax, 0x00(%rdi)
        movq %rbx, 0x08(%rdi)
        movq %rcx, 0x10(%rdi)
        movq %rdx, 0x18(%rdi)
        movq %rsi, 0x20(%rdi)
        movq %rdi, 0x28(%rdi)    # Lưu %rdi (chứa con trỏ context)
        movq %rsp, 0x30(%rdi)
        movq %rbp, 0x38(%rdi)
        movq %r8,  0x40(%rdi)
        movq %r9,  0x48(%rdi)
        movq %r10, 0x50(%rdi)
        movq %r11, 0x58(%rdi)
        movq %r12, 0x60(%rdi)
        movq %r13, 0x68(%rdi)
        movq %r14, 0x70(%rdi)
        movq %r15, 0x78(%rdi)

        # Lưu RIP bằng cách sử dụng call và pop
        call 1f                   # Đặt địa chỉ của nhãn 1 lên stack và gọi nó
1:
        popq %rax                # Lấy địa chỉ trả về từ stack vào %rax
        movq %rax, 0x80(%rdi)    # Lưu RIP vào context

        # Lưu RFLAGS
        pushfq
        popq %rax
        movq %rax, 0x88(%rdi)

        # Lưu CR3
        movq %cr3, %rax
        movq %rax, 0x90(%rdi)

        ret

        .global _switch_to_context
_switch_to_context:
        # Input: %rdi = pointer to cpu_context_t

        # Lưu con trỏ context vào %r11
        movq %rdi, %r11

        # Phục hồi các thanh ghi tổng quát
        movq 0x00(%r11), %rax
        movq 0x08(%r11), %rbx
        movq 0x10(%r11), %rcx
        movq 0x18(%r11), %rdx
        movq 0x20(%r11), %rsi
        movq 0x28(%r11), %rdi
        movq 0x38(%r11), %rbp
        movq 0x40(%r11), %r8
        movq 0x48(%r11), %r9
        movq 0x50(%r11), %r10
        movq 0x60(%r11), %r12
        movq 0x68(%r11), %r13
        movq 0x70(%r11), %r14
        movq 0x78(%r11), %r15

        # Phục hồi RFLAGS
        movq 0x88(%r11), %r12    # Sử dụng %r12 tạm thời
        pushq %r12
        popfq

        # Phục hồi CR3
        movq 0x90(%r11), %r12
        movq %r12, %cr3

        # Phục hồi RSP trước khi phục hồi %r11
        movq 0x30(%r11), %rsp

        # Phục hồi %r11 sau khi hoàn tất việc sử dụng nó
        # Không ghi đè %r11 trước khi hoàn tất việc truy cập bộ nhớ
        # Đã phục hồi %r11 ở trên, nên không cần ghi đè thêm lần nữa

        # Lấy RIP vào %r12
        movq 0x80(%r11), %r12

        # Nhảy đến RIP
        jmp *%r12