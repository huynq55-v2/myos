// paging.c

#include "paging.h"
#include "memory_manager.h"
#include "klibc.h"
#include "graphics.h"
#include "config.h"

#define PAGE_SIZE 4096

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

// Hàm để lấy địa chỉ vật lý từ một entry bảng phân trang
uintptr_t get_physical_address(uint64_t entry)
{
    return entry & 0x000FFFFFFFFFF000;
}

/**
 * Converts a physical address to an entry in a page table.
 *
 * This function takes a physical address and returns a page table entry
 * that points to that address. The returned entry is suitable for use in
 * a page table.
 *
 * @param addr The physical address to convert.
 *
 * @return A page table entry that points to the given physical address.
 */
static uint64_t convert_physical_address_to_page_table_entry(uintptr_t physical_address)
{
    return physical_address | 0x000FFFFFFFFFF000;
}

// Hàm để chuyển đổi chỉ số sang địa chỉ bảng phân trang
static uint64_t *get_table_entry(void *page_table, size_t index)
{
    return (uint64_t *)page_table + index;
}

static inline uintptr_t read_cr3()
{
    uintptr_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

void switch_page_table(void *page_table)
{
    uintptr_t phys_pml4 = VIRT_TO_PHYS(page_table);
    asm volatile("mov %0, %%cr3" : : "r"(phys_pml4));
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
void *create_user_page_table()
{
    // Allocate PML4
    uintptr_t phys_pml4 = allocate_physical_block();
    if (!phys_pml4)
    {
        kprintf("Paging: Failed to allocate PML4\n");
        return NULL;
    }

    // Convert PML4 physical address to virtual
    uint64_t *virt_pml4 = PHYS_TO_VIRT(phys_pml4);

    // Zero PML4
    memset(virt_pml4, 0, 4096);

    // Get current PML4 (kernel)
    uint64_t *current_pml4 = (uint64_t *)PHYS_TO_VIRT(read_cr3());

    // Copy PML4 entries from current PML4 to new PML4
    for (int i = 256; i < 512; i++)
    {
        virt_pml4[i] = current_pml4[i];
    }

    return (void *)phys_pml4;
}

// map memory function
// parameters: PML4, virtual address, physical address, number of bytes, flags
bool map_memory(uint64_t *pml4, uint64_t virt_addr, uint64_t phys_addr, uint64_t size, uint64_t flags)
{
    // calculate number of pages to map
    uint64_t num_pages = size / PAGE_SIZE;
    if (size % PAGE_SIZE != 0)
    {
        num_pages++;
    }

    // map each page
    for (uint64_t i = 0; i < num_pages; i++)
    {
        uint64_t virt_page = virt_addr + i * PAGE_SIZE;
        uint64_t phys_page = phys_addr + i * PAGE_SIZE;

        // calculate page index
        uint64_t pml4_index = virt_page >> 39 & 0x1FF;
        uint64_t pdpt_index = virt_page >> 30 & 0x1FF;
        uint64_t pd_index = virt_page >> 21 & 0x1FF;
        uint64_t pt_index = virt_page >> 12 & 0x1FF;

        // check if pdpt entry is present
        if (!(pml4[pml4_index] & PAGE_PRESENT))
        {
            // allocate pdpt entry
            uint64_t pdpt = allocate_physical_block();
            if (!pdpt)
            {
                kprintf("Paging: Failed to allocate PDPT\n");
                return false;
            }
            // clear pdpt entry
            memset(PHYS_TO_VIRT(pdpt), 0, 4096);
            // set pdpt entry
            pml4[pml4_index] = convert_physical_address_to_page_table_entry(pdpt) | PAGE_PRESENT;
        }

        // get pdpt virtual address
        uint64_t *pdpt = PHYS_TO_VIRT(get_physical_address(pml4[pml4_index]));
        // check if pd entry is present
        if (!(pdpt[pdpt_index] & PAGE_PRESENT))
        {
            // allocate pd entry
            uint64_t pd = allocate_physical_block();
            if (!pd)
            {
                kprintf("Paging: Failed to allocate PD\n");
                return false;
            }
            // clear pd entry
            memset(PHYS_TO_VIRT(pd), 0, 4096);
            // set pd entry
            pdpt[pdpt_index] = convert_physical_address_to_page_table_entry(pd) | PAGE_PRESENT;
        }

        // get pd virtual address
        uint64_t *pd = PHYS_TO_VIRT(get_physical_address(pdpt[pdpt_index]));
        // check if pt entry is present
        if (!(pd[pd_index] & PAGE_PRESENT))
        {
            // allocate pt entry
            uint64_t pt = allocate_physical_block();
            if (!pt)
            {
                kprintf("Paging: Failed to allocate PT\n");
                return false;
            }
            // clear pt entry
            memset(PHYS_TO_VIRT(pt), 0, 4096);
            // set pt entry
            pd[pd_index] = convert_physical_address_to_page_table_entry(pt) | PAGE_PRESENT;
        }

        // get pt virtual address
        uint64_t *pt = PHYS_TO_VIRT(get_physical_address(pd[pd_index]));
        // check if page already mapped
        if (pt[pt_index] & PAGE_PRESENT)
        {
            kprintf("Paging: Page already mapped\n");
            return false;
        }
        // set pt entry
        pt[pt_index] = convert_physical_address_to_page_table_entry(phys_page) | flags;
    }
    return true;
}

// paging.c (tiếp tục)

void destroy_page_table(void *page_table)
{
    if (!page_table)
    {
        kprintf("Paging: Invalid page table\n");
        return;
    }

    uint64_t *pml4 = PHYS_TO_VIRT((uintptr_t)page_table);

    // Lặp qua các entries của PML4
    for (int i = 0; i < 256; i++)
    { // Chỉ xét phần lower half (user space)
        if (pml4[i] & PAGE_PRESENT)
        {
            uintptr_t phys_pdpt = get_physical_address(pml4[i]);
            uint64_t *pdpt = PHYS_TO_VIRT(phys_pdpt);

            // Lặp qua các entries của PDPT
            for (int j = 0; j < 512; j++)
            {
                if (pdpt[j] & PAGE_PRESENT)
                {
                    uintptr_t phys_pd = get_physical_address(pdpt[j]);
                    uint64_t *pd = PHYS_TO_VIRT(phys_pd);

                    // Lặp qua các entries của PD
                    for (int k = 0; k < 512; k++)
                    {
                        if (pd[k] & PAGE_PRESENT)
                        {
                            uintptr_t phys_pt = get_physical_address(pd[k]);
                            uint64_t *pt = PHYS_TO_VIRT(phys_pt);

                            // Lặp qua các entries của PT
                            for (int l = 0; l < 512; l++)
                            {
                                if (pt[l] & PAGE_PRESENT)
                                {
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
