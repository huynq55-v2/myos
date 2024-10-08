// gdt.S

.data
    gdtr:
        .word 0          # Limit storage (2 bytes)
        .quad 0          # Base storage (8 bytes)

.text
.global setGdt
.global reloadSegments
.global loadTss

# Function to set GDTR (setGdt)
setGdt:
    # DI contains limit and RSI contains base address
    movw   %di, gdtr           # Store limit in the first 2 bytes of gdtr
    movq   %rsi, gdtr+2        # Store base address in the next 8 bytes of gdtr
    lgdt   gdtr                # Load GDTR with the new GDT address
    ret                        # Return from function

# Function to reload segment registers (reloadSegments)
reloadSegments:
    pushq $0x08          # Kernel code segment selector (index 1 * 8)
    lea .reload_CS(%rip), %rax
    pushq %rax
    lretq

.reload_CS:
    movw $0x10, %ax      # Kernel data segment selector (index 2 * 8)
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    ret
