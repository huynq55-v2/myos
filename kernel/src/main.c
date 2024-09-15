#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <graphics.h>
#include <font.h>

// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
#if defined (__x86_64__)
        asm ("hlt");
#elif defined (__aarch64__) || defined (__riscv)
        asm ("wfi");
#elif defined (__loongarch64)
        asm ("idle 0");
#endif
    }
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    // Khởi tạo ngữ cảnh đồ họa
    graphics_context_t ctx;
    ctx.framebuffer = (uint32_t *)fb->address;
    ctx.pitch = fb->pitch;
    ctx.width = fb->width;
    ctx.height = fb->height;
    ctx.text_color = 0xFFFFFFFF;       // Màu chữ trắng
    ctx.background_color = 0x00000000; // Màu nền đen
    ctx.cursor_x = 0;
    ctx.cursor_y = 0;
    ctx.line_height = FONT_LINE_HEIGHT;
    ctx.max_rows = ctx.height / ctx.line_height;

    // Xóa màn hình
    for (size_t y = 0; y < ctx.height; y++) {
        for (size_t x = 0; x < ctx.width; x++) {
            put_pixel(&ctx, x, y, ctx.background_color);
        }
    }

    print_text(&ctx, "Hello, World!\n");

    // We're done, just hang...
    hcf();
}
