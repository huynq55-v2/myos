// paging.c

#include "paging.h"
#include "memory_manager.h"
#include "klibc.h"
#include "graphics.h"
#include "config.h"

#define PAGE_SIZE 4096

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))


// Hàm để lấy địa chỉ vật lý từ một entry bảng phân trang
uintptr_t get_physical_address(uint64_t entry) {
    return entry & 0x000FFFFFFFFFF000;
}

// Hàm để đặt địa chỉ vật lý vào một entry bảng phân trang
static uint64_t set_physical_address(uintptr_t addr) {
    return addr & 0x000FFFFFFFFFF000;
}

// Hàm để chuyển đổi chỉ số sang địa chỉ bảng phân trang
static uint64_t* get_table_entry(void *page_table, size_t index) {
    return (uint64_t*)page_table + index;
}

static inline uintptr_t read_cr3()
{
    uintptr_t cr3;
    asm volatile("mov %%cr3, %0" : "=r" (cr3));
    return cr3;
}

void switch_page_table(void *page_table)
{
    uintptr_t phys_pml4 = VIRT_TO_PHYS(page_table);
    asm volatile("mov %0, %%cr3" : : "r" (phys_pml4));
}


/**
 * Creates a new page table for a user process.
 *
 * The function allocates a new physical page for the PML4, converts its
 * physical address to virtual, and initializes it by copying the entries
 * from the current (kernel) page table.
 *
 * @return A pointer to the newly created page table in physical address (i.e. the PML4).
 */
void* create_user_page_table()
{
    // Allocate PML4
    uintptr_t phys_pml4 = allocate_physical_block();
    if (!phys_pml4) {
        kprintf("Paging: Failed to allocate PML4\n");
        return NULL;
    }

    // Convert PML4 physical address to virtual
    uint64_t *virt_pml4 = PHYS_TO_VIRT(phys_pml4);

    // Zero PML4
    memset(virt_pml4, 0, 4096);

    // Get current PML4 (kernel)
    uint64_t *current_pml4 = (uint64_t*)PHYS_TO_VIRT(read_cr3());

    // Copy PML4 entries from current PML4 to new PML4
    for (int i = 256; i < 512; i++) {
        virt_pml4[i] = current_pml4[i];
    }

    return (void*)phys_pml4;
}

bool map_memory(void *page_table, uintptr_t virtual_addr, uintptr_t physical_addr, size_t size, uint64_t flags)
{
    if (!page_table) {
        kprintf("Paging: Invalid page table\n");
        return false;
    }

    uint64_t *pml4 = PHYS_TO_VIRT((uintptr_t)page_table);

    // Tính số trang cần ánh xạ
    size_t pages = ALIGN_UP(size, PAGE_SIZE) / PAGE_SIZE;

    for (size_t i = 0; i < pages; i++) {
        uintptr_t addr = virtual_addr + (i * PAGE_SIZE);
        uintptr_t phys;

        // Nếu physical_addr được cung cấp, sử dụng nó; ngược lại, cấp phát mới
        if (physical_addr != 0) {
            phys = physical_addr + (i * PAGE_SIZE);
        } else {
            phys = allocate_physical_block();
            if (!phys) {
                kprintf("Paging: Failed to allocate physical page\n");
                return false;
            }
        }

        // Trích xuất chỉ số cho từng cấp bảng phân trang
        uint16_t pml4_idx = PML4_INDEX(addr);
        uint16_t pdpt_idx = PDPT_INDEX(addr);
        uint16_t pd_idx   = PD_INDEX(addr);
        uint16_t pt_idx   = PT_INDEX(addr);

        // Kiểm tra và tạo PDPT nếu chưa tồn tại
        if (!(pml4[pml4_idx] & PAGE_PRESENT)) {
            uintptr_t phys_pdpt = allocate_physical_block();
            if (!phys_pdpt) {
                kprintf("Paging: Failed to allocate PDPT\n");
                return false;
            }
            uint64_t *virt_pdpt = PHYS_TO_VIRT(phys_pdpt);
            memset(virt_pdpt, 0, PAGE_SIZE);
            pml4[pml4_idx] = set_physical_address(phys_pdpt) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }

        // Lấy PDPT
        uint64_t *pdpt = PHYS_TO_VIRT(get_physical_address(pml4[pml4_idx]));

        // Kiểm tra và tạo PD nếu chưa tồn tại
        if (!(pdpt[pdpt_idx] & PAGE_PRESENT)) {
            uintptr_t phys_pd = allocate_physical_block();
            if (!phys_pd) {
                kprintf("Paging: Failed to allocate PD\n");
                return false;
            }
            uint64_t *virt_pd = PHYS_TO_VIRT(phys_pd);
            memset(virt_pd, 0, PAGE_SIZE);
            pdpt[pdpt_idx] = set_physical_address(phys_pd) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }

        // Lấy PD
        uint64_t *pd = PHYS_TO_VIRT(get_physical_address(pdpt[pdpt_idx]));

        // Kiểm tra và tạo PT nếu chưa tồn tại
        if (!(pd[pd_idx] & PAGE_PRESENT)) {
            uintptr_t phys_pt = allocate_physical_block();
            if (!phys_pt) {
                kprintf("Paging: Failed to allocate PT\n");
                return false;
            }
            uint64_t *virt_pt = PHYS_TO_VIRT(phys_pt);
            memset(virt_pt, 0, PAGE_SIZE);
            pd[pd_idx] = set_physical_address(phys_pt) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        }

        // Lấy PT
        uint64_t *pt = PHYS_TO_VIRT(get_physical_address(pd[pd_idx]));

        // Kiểm tra nếu trang đã được ánh xạ
        if (pt[pt_idx] & PAGE_PRESENT) {
            kprintf("Paging: Virtual address 0x%lx is already mapped\n", addr);
            if (physical_addr != 0) {
                // Nếu đã cung cấp physical_addr và trang đã được ánh xạ, có thể là lỗi
                return false;
            }
            continue; // Nếu tự cấp phát và trang đã được map, bỏ qua
        }

        // Ánh xạ trang
        pt[pt_idx] = set_physical_address(phys) | flags | PAGE_PRESENT;

        // Zero trang mới
        memset(PHYS_TO_VIRT(phys), 0, PAGE_SIZE);
    }

    return true;
}


// paging.c (tiếp tục)

void destroy_page_table(void *page_table)
{
    if (!page_table) {
        kprintf("Paging: Invalid page table\n");
        return;
    }

    uint64_t *pml4 = PHYS_TO_VIRT((uintptr_t)page_table);

    // Lặp qua các entries của PML4
    for (int i = 0; i < 256; i++) { // Chỉ xét phần lower half (user space)
        if (pml4[i] & PAGE_PRESENT) {
            uintptr_t phys_pdpt = get_physical_address(pml4[i]);
            uint64_t *pdpt = PHYS_TO_VIRT(phys_pdpt);

            // Lặp qua các entries của PDPT
            for (int j = 0; j < 512; j++) {
                if (pdpt[j] & PAGE_PRESENT) {
                    uintptr_t phys_pd = get_physical_address(pdpt[j]);
                    uint64_t *pd = PHYS_TO_VIRT(phys_pd);

                    // Lặp qua các entries của PD
                    for (int k = 0; k < 512; k++) {
                        if (pd[k] & PAGE_PRESENT) {
                            uintptr_t phys_pt = get_physical_address(pd[k]);
                            uint64_t *pt = PHYS_TO_VIRT(phys_pt);

                            // Lặp qua các entries của PT
                            for (int l = 0; l < 512; l++) {
                                if (pt[l] & PAGE_PRESENT) {
                                    uintptr_t phys_page = get_physical_address(pt[l]);
                                    free_physical_block(phys_page); // Giải phóng trang vật lý
                                }
                            }

                            // Giải phóng PT
                            free_physical_block(phys_pd);
                        }
                    }

                    // Giải phóng PDPT
                    free_physical_block(phys_pdpt);
                }
            }

            // Đặt entry trong PML4 thành 0
            pml4[i] = 0;
        }
    }

    // Giải phóng PML4
    uintptr_t phys_pml4 = VIRT_TO_PHYS(page_table);
    free_physical_block(phys_pml4);
}
