#include "graphics.h"
#include "font.h"  // Để truy cập dữ liệu font như `roboto_glyphs`
#include "limine.h"
#include "klibc.h"

static graphics_context_t g_ctx;

// Hàm khởi tạo graphics context
void init_graphics(struct limine_framebuffer *fb) {
    g_ctx.framebuffer = (uint32_t *)fb->address;
    g_ctx.pitch = fb->pitch;
    g_ctx.width = fb->width;
    g_ctx.height = fb->height;
    g_ctx.text_color = 0xFFFFFFFF;       // Màu chữ trắng
    g_ctx.background_color = 0x00000000; // Màu nền đen
    g_ctx.cursor_x = 0;
    g_ctx.cursor_y = 0;
    g_ctx.prev_cursor_x = 0;
    g_ctx.prev_cursor_y = 0;
    g_ctx.line_height = FONT_LINE_HEIGHT;
    g_ctx.max_rows = g_ctx.height / g_ctx.line_height;
}

void put_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && (size_t)x < g_ctx.width && y >= 0 && (size_t)y < g_ctx.height) {
        g_ctx.framebuffer[y * (g_ctx.pitch / 4) + x] = color;
    }
}

void draw_glyph(int x, int y, char c) {
    if (c < 32 || c > 126) {
        c = '?';
    }

    const glyph_t *glyph = &roboto_glyphs[c - 32];
    const uint8_t *data = glyph->data;

    int glyph_x = x + glyph->x_offset;
    int glyph_y = y + glyph->y_offset + FONT_ASCENT;

    for (int row = 0; row < glyph->height; row++) {
        for (int col = 0; col < glyph->width; col++) {
            uint8_t pixel = data[row * glyph->width + col];
            if (pixel > 128) {
                int px = glyph_x + col;
                int py = glyph_y + row;
                put_pixel(px, py, g_ctx.text_color);
            }
        }
    }
}

void draw_text(int x, int y, const char *text) {
    g_ctx.cursor_x = x;
    g_ctx.cursor_y = y;

    while (*text) {
        if (*text == '\n') {
            g_ctx.cursor_x = x;
            g_ctx.cursor_y += g_ctx.line_height;
        } else {
            draw_glyph(g_ctx.cursor_x, g_ctx.cursor_y, *text);
            const glyph_t *glyph = &roboto_glyphs[*text - 32];
            g_ctx.cursor_x += glyph->x_advance;
        }
        text++;
    }
}

void print_text(const char *text) {
    // Xóa dấu nháy con trỏ cũ
    erase_cursor(g_ctx);

    while (*text) {
        if (*text == '\n') {
            // Di chuyển con trỏ đến đầu dòng mới
            g_ctx.cursor_x = 0;
            g_ctx.cursor_y += g_ctx.line_height;

            // Kiểm tra nếu cần cuộn màn hình
            if ((size_t)(g_ctx.cursor_y + g_ctx.line_height) > g_ctx.height) {
                scroll_screen();
                g_ctx.cursor_y -= g_ctx.line_height;
            }
        } else {
            // Vẽ ký tự
            draw_glyph(g_ctx.cursor_x, g_ctx.cursor_y, *text);

            // Lấy glyph để biết x_advance
            const glyph_t *glyph = &roboto_glyphs[*text - 32];
            g_ctx.cursor_x += glyph->x_advance;

            // Kiểm tra nếu con trỏ vượt quá chiều rộng màn hình
            if ((size_t)(g_ctx.cursor_x + glyph->width) > g_ctx.width) {
                // Di chuyển con trỏ đến đầu dòng mới
                g_ctx.cursor_x = 0;
                g_ctx.cursor_y += g_ctx.line_height;

                // Kiểm tra nếu cần cuộn màn hình
                if ((size_t)(g_ctx.cursor_y + g_ctx.line_height) > g_ctx.height) {
                    scroll_screen();
                    g_ctx.cursor_y -= g_ctx.line_height;
                }
            }
        }
        text++;
    }

    // Vẽ dấu nháy con trỏ mới
    draw_cursor();
}

void scroll_screen() {
    // Tính số byte mỗi dòng
    size_t row_size = g_ctx.pitch * g_ctx.line_height;

    // Tính số byte cần di chuyển (từ dòng thứ hai đến dòng cuối)
    size_t move_size = g_ctx.pitch * (g_ctx.height - g_ctx.line_height);

    // Di chuyển vùng nhớ lên trên một dòng
    memmove(
        g_ctx.framebuffer,
        (uint8_t *)g_ctx.framebuffer + row_size,
        move_size
    );

    // Xóa dòng cuối cùng
    size_t last_row_offset = g_ctx.pitch * (g_ctx.height - g_ctx.line_height) / 4;
    for (size_t y = 0; y < (size_t)g_ctx.line_height; y++) {
        for (size_t x = 0; x < g_ctx.width; x++) {
            g_ctx.framebuffer[last_row_offset + y * (g_ctx.pitch / 4) + x] = g_ctx.background_color;
        }
    }
}

void erase_cursor() {
    int cursor_width = 2;
    int cursor_height = g_ctx.line_height;

    for (int y = 0; y < cursor_height; y++) {
        for (int x = 0; x < cursor_width; x++) {
            int px = g_ctx.prev_cursor_x + x;
            int py = g_ctx.prev_cursor_y + y;
            if (px >= 0 && (size_t)px < g_ctx.width && py >= 0 && (size_t)py < g_ctx.height) {
                g_ctx.framebuffer[py * (g_ctx.pitch / 4) + px] = g_ctx.background_color;
            }
        }
    }
}

void draw_cursor() {
    int cursor_width = 2;
    int cursor_height = g_ctx.line_height;

    // Cập nhật vị trí con trỏ cũ trước khi vẽ dấu nháy mới
    g_ctx.prev_cursor_x = g_ctx.cursor_x;
    g_ctx.prev_cursor_y = g_ctx.cursor_y;

    for (int y = 0; y < cursor_height; y++) {
        for (int x = 0; x < cursor_width; x++) {
            int px = g_ctx.cursor_x + x;
            int py = g_ctx.cursor_y + y;
            if (px >= 0 && (size_t)px < g_ctx.width && py >= 0 && (size_t)py < g_ctx.height) {
                g_ctx.framebuffer[py * (g_ctx.pitch / 4) + px] = g_ctx.text_color;
            }
        }
    }
}

void print(const char *text) {
    print_text(text);
}

// Hàm chuyển đổi số nguyên thành chuỗi
void itoa(int value, char *str, int base) {
    char *digits = "0123456789ABCDEF";
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    int tmp_value;

    // Xử lý số âm trong hệ thập phân (base 10)
    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
    }

    // Chuyển đổi số thành chuỗi
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = digits[tmp_value % base];
    } while (value);

    // Thêm ký tự kết thúc chuỗi
    *ptr-- = '\0';

    // Đảo ngược chuỗi
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

#include <stdarg.h>

void itoa_unsigned(uint64_t value, char *str, int base) {
    char *digits = "0123456789ABCDEF";
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    uint64_t tmp_value;

    // Chuyển đổi số thành chuỗi
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = digits[tmp_value % base];
    } while (value);

    // Thêm ký tự kết thúc chuỗi
    *ptr-- = '\0';

    // Đảo ngược chuỗi
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

void kprintf(const char *format, ...) {
    char buffer[64];  // Tăng kích thước bộ đệm
    const char *ptr;
    va_list args;
    va_start(args, format);

    // Duyệt qua từng ký tự trong chuỗi format
    for (ptr = format; *ptr != '\0'; ptr++) {
        if (*ptr == '%') {
            ptr++;
            switch (*ptr) {
                case 'd': {
                    // In số nguyên thập phân
                    int value = va_arg(args, int);
                    itoa(value, buffer, 10);
                    print_text(buffer);
                    break;
                }
                case 'u': {
                    // In số nguyên không dấu
                    unsigned int value = va_arg(args, unsigned int);
                    itoa_unsigned(value, buffer, 10);
                    print_text(buffer);
                    break;
                }
                case 'x': {
                    // In số nguyên hệ thập lục phân
                    unsigned int value = va_arg(args, unsigned int);
                    itoa_unsigned(value, buffer, 16);
                    print_text("0x");
                    print_text(buffer);
                    break;
                }
                case 'p': {
                    // In địa chỉ bộ nhớ (con trỏ)
                    void *ptr_value = va_arg(args, void*);
                    uint64_t addr = (uint64_t)ptr_value;
                    itoa_unsigned(addr, buffer, 16);
                    print_text("0x");
                    print_text(buffer);
                    break;
                }
                case 'c': {
                    // In ký tự
                    char c = (char)va_arg(args, int);
                    char str[2] = {c, '\0'};
                    print_text(str);
                    break;
                }
                case 's': {
                    // In chuỗi ký tự
                    char *str = va_arg(args, char*);
                    print_text(str);
                    break;
                }
                case '%': {
                    // In ký tự '%'
                    print_text("%");
                    break;
                }
                default:
                    // Nếu không phải định dạng hợp lệ, in trực tiếp ký tự
                    print_text("%");
                    print_text((char[]){*ptr, '\0'});
                    break;
            }
        } else {
            // In trực tiếp các ký tự không phải là định dạng
            print_text((char[]){*ptr, '\0'});
        }
    }

    va_end(args);
}
