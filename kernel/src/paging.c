// paging.c

#include "paging.h"
#include "memory_manager.h"
#include "klibc.h"
#include "graphics.h"
#include "config.h"

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

#define PML4_INDEX(x) (((x) >> 39) & 0x1FF)
#define PDPT_INDEX(x) (((x) >> 30) & 0x1FF)
#define PD_INDEX(x)   (((x) >> 21) & 0x1FF)
#define PT_INDEX(x)   (((x) >> 12) & 0x1FF)

/**
 * Reads the current value of CR3.
 *
 * This function reads the current value of the CR3 register and returns it as a
 * uintptr_t. The CR3 register contains the physical address of the base of the
 * page directory associated with the current process.
 *
 * @return The physical address of the page directory base.
 */
static inline uintptr_t read_cr3()
{
    uintptr_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

// Switches the current page table.
// page_table is virtual address
void switch_page_table(void *page_table)
{
    asm volatile("mov %0, %%cr3" : : "r"(page_table));
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

/**
 * Maps a range of physical memory to a range of virtual memory.
 *
 * The function takes a pointer to the PML4, a virtual address, a physical address,
 * a size in bytes, and flags as parameters. It maps the given range of physical
 * memory to the given range of virtual memory, and sets the given flags for the
 * mapping.
 *
 * The function returns true if the mapping was successful, and false otherwise.
 *
 * @param pml4 A pointer to the PML4.
 * @param virt_addr The virtual address of the mapping.
 * @param phys_addr The physical address of the mapping.
 * @param size The size of the mapping in bytes.
 * @param flags The flags for the mapping.
 *
 * @return true if the mapping was successful, and false otherwise.
 */
bool map_memory(uintptr_t pml4_phys, uint64_t virt_addr, uint64_t phys_addr, uint64_t size, uint64_t flags)
{
    // check if virtual address and physical address are aligned
    if (virt_addr % PAGE_SIZE != 0 || phys_addr % PAGE_SIZE != 0)
    {
        kprintf("Paging: Virtual and physical addresses must be aligned\n");
        return false;
    }

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

        uint64_t *pml4_virtual = PHYS_TO_VIRT((uintptr_t)pml4_phys);

        if (!(pml4_virtual[pml4_index] & PAGING_PAGE_PRESENT))
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
            pml4_virtual[pml4_index] = pdpt & 0xFFFFFFFFFFFFF000 | flags | PAGING_PAGE_PRESENT;
        }

        // get pdpt virtual address
        uint64_t *pdpt = PHYS_TO_VIRT(pml4_virtual[pml4_index] & 0xFFFFFFFFFFFFF000);
        // check if pd entry is present
        if (!(pdpt[pdpt_index] & PAGING_PAGE_PRESENT))
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
            pdpt[pdpt_index] = pd & 0xFFFFFFFFFFFFF000 | flags | PAGING_PAGE_PRESENT;
        }

        // get pd virtual address
        uint64_t *pd = PHYS_TO_VIRT(pdpt[pdpt_index] & 0xFFFFFFFFFFFFF000);
        // check if pt entry is present
        if (!(pd[pd_index] & PAGING_PAGE_PRESENT))
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
            pd[pd_index] = pt & 0xFFFFFFFFFFFFF000 | flags | PAGING_PAGE_PRESENT;
        }

        // get pt virtual address
        uint64_t *pt = PHYS_TO_VIRT(pd[pd_index] & 0xFFFFFFFFFFFFF000);
        // check if page already mapped
        if (pt[pt_index] & PAGING_PAGE_PRESENT)
        {
            kprintf("Paging: Page already mapped\n");
            return false;
        }
        // set pt entry
        pt[pt_index] = phys_page & 0xFFFFFFFFFFFFF000 | flags | PAGING_PAGE_PRESENT;

        // Invalidate TLB for the mapped page
        __asm__ volatile("invlpg (%0)" : : "r" (virt_page) : "memory");
    }
    return true;
}
