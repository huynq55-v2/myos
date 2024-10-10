#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"
#include "graphics.h"
#include "font.h"
#include "idt.h"
#include "gdt.h"
#include "tss.h"
#include "memory_manager.h"
#include "process.h"
#include "apic.h"

#ifdef TEST
void run_all_tests();
#endif

// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0};

// Thêm yêu cầu MEMMAP
__attribute__((used, section(".requests"))) volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function.
static void hcf(void)
{
    for (;;)
    {
#if defined(__x86_64__)
        asm("hlt");
#elif defined(__aarch64__) || defined(__riscv)
        asm("wfi");
#elif defined(__loongarch64)
        asm("idle 0");
#endif
    }
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void)
{
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
    {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    // Bật OSFXSR và OSXMMEXCPT bằng inline assembly
    __asm__ __volatile__ (
        "mov %%cr4, %%rax\n\t"
        "or $0x600, %%rax\n\t"
        "mov %%rax, %%cr4"
        :
        :
        : "rax"
    );

    // Khởi tạo graphics context
    init_graphics(fb);
    init_gdt();
    idt_init();  // Nạp IDT

    memory_manager_init();

    scheduler_init();  // Khởi tạo bộ lập lịch và tiến trình đầu tiên

    // Khởi tạo bộ hẹn giờ APIC để ngắt mỗi 10 ms (100 Hz)
    apic_timer_init(100);

    __asm__ volatile ("sti");  // Bật cờ ngắt toàn cục

    // scheduler_init();  // Khởi tạo bộ lập lịch và tiến trình đầu tiên
    // scheduler_start(); // Bắt đầu bộ lập lịch

    // timer_interrupt_handler_c();

    hcf();  // Dừng hệ thống trong khi chờ đợi
}
