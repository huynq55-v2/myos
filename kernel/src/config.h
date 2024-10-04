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

#endif // CONFIG_H
