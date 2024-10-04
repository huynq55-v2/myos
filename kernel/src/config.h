// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// Định nghĩa offset cho High Half Direct Mapping (HHDM)
#define HHDM_OFFSET 0xFFFF800000000000

// Macro để chuyển đổi từ địa chỉ vật lý sang địa chỉ ảo
#define PHYS_TO_VIRT(x) ((void *)((uintptr_t)(x) + HHDM_OFFSET))

// Macro để chuyển đổi từ địa chỉ ảo sang địa chỉ vật lý
#define VIRT_TO_PHYS(x) ((void *)((uintptr_t)(x) - HHDM_OFFSET))

#define BLOCK_SIZE 4096

#define PAGE_SIZE 4096

// Các hằng số cờ phân trang
#define PAGING_PAGE_PRESENT    0x1
#define PAGING_PAGE_RW         0x2
#define PAGING_PAGE_USER       0x4

// Hằng số phân quyền cụ thể
#define PERMISSION_READ        0x1
#define PERMISSION_WRITE       0x2
#define PERMISSION_EXECUTE     0x4

#endif // CONFIG_H
