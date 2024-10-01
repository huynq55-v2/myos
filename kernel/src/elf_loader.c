// elf_loader.c

#include "elf_loader.h"
#include "elf.h"
#include "buddy.h"
#include "klibc.h"
#include "process.h"
#include "graphics.h"

// Hàm để tải ELF vào bộ nhớ
bool load_elf(const uint8_t *elf_data, size_t size, process_t *proc)
{
    if (size < sizeof(Elf64_Ehdr))
    {
        kprintf("ELF: File too small\n");
        return false;
    }

    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_data;

    // Kiểm tra magic number của ELF
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0)
    {
        kprintf("ELF: Invalid magic number\n");
        return false;
    }

    // Chỉ hỗ trợ ELF 64-bit
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64)
    {
        kprintf("ELF: Unsupported ELF class\n");
        return false;
    }

    // Chỉ hỗ trợ little endian
    if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
    {
        kprintf("ELF: Unsupported endianness\n");
        return false;
    }

    // Duyệt qua các program header
    Elf64_Phdr *phdr = (Elf64_Phdr *)(elf_data + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        Elf64_Phdr *ph = &phdr[i];

        if (ph->p_type != PT_LOAD)
            continue;

        // Địa chỉ ảo nơi segment sẽ được nạp
        void *segment_virtual_addr = (void *)(ph->p_vaddr);

        // Kích thước cần cấp phát
        size_t memsz = ph->p_memsz;

        // Cấp phát bộ nhớ cho segment (cần triển khai hàm map_memory)
        if (!map_memory(proc->page_table, (uintptr_t)segment_virtual_addr, memsz, ph->p_flags))
        {
            kprintf("ELF: Failed to map memory for segment\n");
            return false;
        }

        // Sao chép dữ liệu từ ELF vào bộ nhớ đã cấp phát
        memcpy(segment_virtual_addr, elf_data + ph->p_offset, ph->p_filesz);

        // Nếu p_memsz > p_filesz, điền zero vào phần còn lại
        if (ph->p_memsz > ph->p_filesz)
        {
            memset((uint8_t *)segment_virtual_addr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
        }
    }

    // Thiết lập điểm bắt đầu cho tiến trình
    proc->entry_point = ehdr->e_entry;

    return true;
}
