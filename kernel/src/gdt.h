// gdt.h

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Định nghĩa GDTEntry32
typedef struct
{
    uint16_t limit_low;       // Bits 0-15 của limit
    uint16_t base_low;        // Bits 0-15 của base
    uint8_t base_middle;      // Bits 16-23 của base
    uint8_t access;           // Access flags
    uint8_t granularity;      // Granularity và bits 16-19 của limit
    uint8_t base_high;        // Bits 24-31 của base
} __attribute__((packed)) GDTEntry32;

// Định nghĩa GDTEntry64 cho TSS và LDT
typedef struct
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle1;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_middle2;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__((packed)) GDTEntry64;

// Định nghĩa cấu trúc GDT
typedef struct
{
    GDTEntry32 entries32[5];
    GDTEntry64 tss_entry;
} __attribute__((packed)) GDT;

// Định nghĩa GDTR
typedef struct
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) GDTR;

// Khai báo các biến toàn cục
extern GDT gdt;            // Biến GDT
extern GDTR gdt_ptr;       // GDTR

// Hàm khởi tạo GDT
void init_gdt();

// Các hàm hỗ trợ
extern void setGdt(uint16_t limit, uint64_t base);
extern void reloadSegments(void);

#endif // GDT_H
