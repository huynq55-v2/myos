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

extern uint8_t hello_user_elf_start[];
extern uint8_t hello_user_elf_end[];

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
    idt_init(); // Nạp IDT

    memory_manager_init();

#ifdef TEST
    run_all_tests();
    hcf();
#else
    // Tạo tiến trình đầu tiên (ví dụ: init process)
    process_t *init_proc = process_create(hello_user_elf_start, hello_user_elf_end, 0);
    if (!init_proc) {
        kprintf("Kernel: Failed to create init process\n");
        while (1) {}
    }

    // Thêm tiến trình đầu tiên vào hàng đợi sẵn sàng
    process_enqueue(init_proc);

    // Vòng lặp chính của scheduler
    while (1) {
        process_run();
    }

    // We're done, just hang...
    hcf();
#endif
}
