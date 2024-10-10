// tests/tests.c
#include "graphics.h"
#include <stdbool.h>
#include "gdt.h"
#include "tss.h"
#include "stacks.h"
#include "tests.h"
#include "idt.h"

// Hàm để in kết quả kiểm thử
void test_print_result(const char *test_name, bool result) {
    if (result) {
        kprintf("PASS: %s\n", test_name);
    } else {
        kprintf("FAIL: %s\n", test_name);
    }
}

// // Kiểm thử GDT: kiểm tra xem GDT đã được nạp đúng chưa
// void test_gdt() {
//     bool result = true;

//     // Kiểm tra giới hạn của GDTR
//     if (gdt_ptr.limit != sizeof(GDTEntry32) * 5 + sizeof(GDTEntry64) - 1) {
//         result = false;
//     }

//     test_print_result("GDT Initialization Test", result);
// }

// Kiểm thử các mục trong GDT: kiểm tra access flags và granularity
// void test_gdt_entries() {
//     bool result = true;
//     GDTEntry32 *entries32 = (GDTEntry32 *)gdt;

//     // Kiểm tra các mục code và data segments
//     if ((entries32[0].access & 0xFE) != 0x00) { // Null segment
//         result = false;
//     }
//     if ((entries32[1].access & 0xFE) != 0x9A) { // Kernel code segment
//         result = false;
//     }
//     if ((entries32[2].access & 0xFE) != 0x92) { // Kernel data segment
//         result = false;
//     }
//     if ((entries32[3].access & 0xFE) != 0xFA) { // User code segment
//         result = false;
//     }
//     if ((entries32[4].access & 0xFE) != 0xF2) { // User data segment
//         result = false;
//     }

//     // Kiểm tra granularity của các mục
//     if ((entries32[1].granularity & 0xF0) != 0x20) { // Kernel code segment
//         result = false;
//     }
//     if ((entries32[2].granularity & 0xF0) != 0x00) { // Kernel data segment
//         result = false;
//     }
//     if ((entries32[3].granularity & 0xF0) != 0x20) { // User code segment
//         result = false;
//     }
//     if ((entries32[4].granularity & 0xF0) != 0x00) { // User data segment
//         result = false;
//     }

//     test_print_result("GDT Entries Test", result);
// }

// Kiểm thử TSS: kiểm tra xem TSS đã được nạp đúng chưa
void test_tss() {
    bool result = true;

    // Kiểm tra xem stack pointer cho kernel có được khởi tạo đúng không
    if (tss.rsp0 != kernel_stack_top) {
        result = false;
    }

    // Kiểm tra xem IST1 có được khởi tạo đúng không
    if (tss.ist1 != ist1_stack_top) {
        result = false;
    }

    // Kiểm tra I/O map base
    if (tss.iomap_base != sizeof(TSS)) {
        result = false;
    }

    test_print_result("TSS Initialization Test", result);
}

// void test_tss_descriptor() {
//     bool result = true;
//     GDTEntry64 *tss_entry = (GDTEntry64 *)(gdt + sizeof(GDTEntry32) * 5);

//     // Kiểm tra các trường của Descriptor TSS
//     uint64_t expected_base = (uint64_t)&tss;
//     uint32_t expected_limit = sizeof(TSS);

//     // Kiểm tra trường access: 0x89 (Available) hoặc 0x8B (Busy)
//     if (tss_entry->access != 0x89 && tss_entry->access != 0x8B) {
//         result = false;
//         kprintf("3\n");
//         kprintf("Expected access: 0x89 or 0x8B, Actual access: 0x%x\n", tss_entry->access);
//     }

//     // Kiểm tra các trường khác nếu cần
//     if (tss_entry->limit_low != (expected_limit & 0xFFFF)) {
//         result = false;
//         kprintf("Limit low condition failed: 0x%x != 0x%x\n", tss_entry->limit_low, (expected_limit & 0xFFFF));
//     }
//     if (tss_entry->base_low != (expected_base & 0xFFFF)) {
//         result = false;
//         kprintf("Base low condition failed: 0x%x != 0x%x\n", tss_entry->base_low, (expected_base & 0xFFFF));
//     }
//     if (tss_entry->base_middle1 != ((expected_base >> 16) & 0xFF)) {
//         result = false;
//         kprintf("Base middle1 condition failed: 0x%x != 0x%x\n", tss_entry->base_middle1, ((expected_base >> 16) & 0xFF));
//     }
//     if (tss_entry->granularity != ((expected_limit >> 16) & 0x0F)) {
//         result = false;
//         kprintf("Granularity condition failed: 0x%x != 0x%x\n", tss_entry->granularity, ((expected_limit >> 16) & 0x0F));
//     }
//     if (tss_entry->base_middle2 != ((expected_base >> 24) & 0xFF)) {
//         result = false;
//         kprintf("Base middle2 condition failed: 0x%x != 0x%x\n", tss_entry->base_middle2, ((expected_base >> 24) & 0xFF));
//     }
//     if (tss_entry->base_high != (expected_base >> 32)) {
//         result = false;
//         kprintf("Base high condition failed: 0x%x != 0x%lx\n", tss_entry->base_high, (expected_base >> 32));
//     }
//     if (tss_entry->reserved != 0) {
//         result = false;
//         kprintf("Reserved condition failed: 0x%x != 0x0\n", tss_entry->reserved);
//     }

//     test_print_result("TSS Descriptor Test", result);
// }

// Kiểm thử IDT: kiểm tra xem IDT đã được nạp đúng chưa
void test_idt_initialization() {
    bool result = true;

    // Kiểm tra giới hạn của IDTR
    if (idtr.limit != (sizeof(idt_entry_t) * IDT_SIZE) - 1) {
        result = false;
    }

    // Kiểm tra base của IDTR
    if (idtr.base != (uint64_t)&idt) {
        result = false;
    }

    test_print_result("IDT Initialization Test", result);
}

// Kiểm thử các mục trong IDT: kiểm tra selector và type_attr của các gate
void test_idt_entries() {
    bool result = true;

    // Kiểm tra các entry từ vector 0 đến 31
    for (int i = 0; i < 32; i++) {
        // Selector phải là 0x08 (kernel code segment)
        if (idt[i].selector != 0x08) {
            result = false;
            break;
        }

        // Type_attr phải là 0x8E (interrupt gate, present, privilege level 0)
        if (idt[i].type_attr != 0x8E) {
            result = false;
            break;
        }

        // IST phải là 0 đối với hầu hết các entry trừ vector 8 (double fault)
        if (i == 8) {
            if (idt[i].ist != 1) { // IST1 được sử dụng cho double fault
                result = false;
                break;
            }
        } else {
            if (idt[i].ist != 0) {
                result = false;
                break;
            }
        }

        // Offset của handler phải đúng với địa chỉ của ISR tương ứng
        // Giả sử rằng isr_table được sắp xếp theo vector_num
        extern void *isr_table[];
        if (idt[i].offset_low != ((uint64_t)isr_table[i] & 0xFFFF)) {
            result = false;
            break;
        }
        if (idt[i].offset_mid != (((uint64_t)isr_table[i] >> 16) & 0xFFFF)) {
            result = false;
            break;
        }
        if (idt[i].offset_high != (((uint64_t)isr_table[i] >> 32) & 0xFFFFFFFF)) {
            result = false;
            break;
        }
    }

    test_print_result("IDT Entries Test", result);
}

// Kiểm thử các stack: kiểm tra xem các stack đã được thiết lập đúng không
void test_stacks() {
    bool result = true;

    // Kiểm tra địa chỉ của kernel_stack_top
    if (kernel_stack_top != (uint64_t)(kernel_stack + sizeof(kernel_stack))) {
        result = false;
    }

    // Kiểm tra địa chỉ của ist1_stack_top
    if (ist1_stack_top != (uint64_t)(ist1_stack + sizeof(ist1_stack))) {
        result = false;
    }

    test_print_result("Stacks Initialization Test", result);
}

// Hàm chạy tất cả kiểm thử
void run_all_tests() {
    kprintf("=== Starting All Tests ===\n");

    // test_gdt();
    // test_gdt_entries();
    test_tss();
    // test_tss_descriptor();
    test_idt_initialization();
    test_idt_entries();
    test_stacks();

    kprintf("=== All Tests Completed ===\n");
}
