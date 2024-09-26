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
    # Reload CS register:
    pushq $0x08                # Push code segment selector (0x08) onto the stack
    lea   .reload_CS(%rip), %rax  # Load address of .reload_CS into RAX
    pushq %rax                 # Push this value onto the stack
    lretq                      # Perform far return to reload CS

.reload_CS:
    # Reload data segment registers
    movw  $0x10, %ax           # Data segment selector (0x10)
    movw  %ax, %ds
    movw  %ax, %es
    movw  %ax, %fs
    movw  %ax, %gs
    movw  %ax, %ss
    ret                        # Return after reloading the segments

# Function to load TSS (loadTss)
loadTss:
    # AX contains the TSS selector
    ltr %ax
    ret
