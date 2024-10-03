/* linker.ld */

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

/* Định nghĩa địa chỉ ảo và vật lý */
KERNEL_VMA = 0xFFFFFFFF80000000; /* Địa chỉ ảo nơi kernel được liên kết */
KERNEL_LMA = 0x100000;           /* Địa chỉ vật lý nơi kernel được nạp */

SECTIONS
{
    /* Phần .text của kernel */
    .text KERNEL_VMA : AT(KERNEL_LMA) {
        _kernel_start = .;          /* Đánh dấu bắt đầu của phần .text */
        *(.multiboot_header)
        *(.text*)
        *(.rodata*)
        _kernel_end = .;            /* Đánh dấu kết thúc của phần .text */
    }

    /* Phần .data của kernel */
    .data : AT(KERNEL_LMA + (ADDR(.data) - KERNEL_VMA)) {
        *(.data*)
    }

    /* Phần .bss của kernel */
    .bss : AT(KERNEL_LMA + (ADDR(.bss) - KERNEL_VMA)) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    /* Bao gồm ELF của chương trình người dùng */
    .user_program : AT(KERNEL_LMA + (ADDR(.user_program) - KERNEL_VMA)) ALIGN(0x1000) {
        hello_user_elf_start = .;
        KEEP(*(.user_program))
        hello_user_elf_end = .;
    }

    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
    }
}
