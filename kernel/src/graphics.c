#include "graphics.h"
#include "font.h"  // Để truy cập dữ liệu font như `roboto_glyphs`
#include "limine.h"
#include "klibc.h"
#include <stdarg.h>

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
    // erase_cursor(g_ctx);

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
    // draw_cursor();
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

// Hàm chuyển đổi số nguyên có dấu thành chuỗi
void itoa(int64_t value, char *str, int base) {
    char *digits = "0123456789abcdef";
    char buffer[65];
    char *ptr = &buffer[64];
    int is_negative = 0;

    *ptr = '\0';

    if (value == 0) {
        *--ptr = '0';
    } else {
        if (value < 0) {
            is_negative = 1;
            value = -value;
        }
        while (value != 0) {
            *--ptr = digits[value % base];
            value /= base;
        }
        if (is_negative) {
            *--ptr = '-';
        }
    }

    // Sao chép kết quả vào str
    while (*ptr) {
        *str++ = *ptr++;
    }
    *str = '\0';
}

// Hàm chuyển đổi số nguyên không dấu thành chuỗi
void utoa(uint64_t value, char *str, int base) {
    char *digits = "0123456789abcdef";
    char buffer[65];
    char *ptr = &buffer[64];

    *ptr = '\0';

    if (value == 0) {
        *--ptr = '0';
    } else {
        while (value != 0) {
            *--ptr = digits[value % base];
            value /= base;
        }
    }

    // Sao chép kết quả vào str
    while (*ptr) {
        *str++ = *ptr++;
    }
    *str = '\0';
}

void kprintf(const char *format, ...) {
    char buffer[128];
    const char *fmt_ptr;
    va_list args;
    va_start(args, format);

    for (fmt_ptr = format; *fmt_ptr != '\0'; fmt_ptr++) {
        if (*fmt_ptr == '%') {
            fmt_ptr++;

            // Handle length modifiers
            char length = '\0';
            if (*fmt_ptr == 'l' || *fmt_ptr == 'h') {
                length = *fmt_ptr++;
                if (*fmt_ptr == 'l' || *fmt_ptr == 'h') {
                    // Support 'll' and 'hh' if needed
                    length = *fmt_ptr++;
                }
            }

            char specifier = *fmt_ptr;

            switch (specifier) {
                case 'd':
                case 'i': {
                    int64_t value;
                    if (length == 'l') {
                        value = va_arg(args, long);
                    } else if (length == 'h') {
                        value = (short)va_arg(args, int);
                    } else {
                        value = va_arg(args, int);
                    }
                    itoa(value, buffer, 10);
                    print_text(buffer);
                    break;
                }
                case 'u': {
                    uint64_t value;
                    if (length == 'l') {
                        value = va_arg(args, unsigned long);
                    } else if (length == 'h') {
                        value = (unsigned short)va_arg(args, unsigned int);
                    } else {
                        value = va_arg(args, unsigned int);
                    }
                    utoa(value, buffer, 10);
                    print_text(buffer);
                    break;
                }
                case 'x':
                case 'X': {
                    uint64_t value;
                    if (length == 'l') {
                        value = va_arg(args, unsigned long);
                    } else if (length == 'h') {
                        value = (unsigned short)va_arg(args, unsigned int);
                    } else {
                        value = va_arg(args, unsigned int);
                    }
                    utoa(value, buffer, 16);
                    if (specifier == 'X') {
                        // Convert letters to uppercase
                        for (char *p = buffer; *p; p++) {
                            if (*p >= 'a' && *p <= 'f') {
                                *p -= 32;
                            }
                        }
                    }
                    print_text("0x");
                    print_text(buffer);
                    break;
                }
                case 'o': {
                    uint64_t value;
                    if (length == 'l') {
                        value = va_arg(args, unsigned long);
                    } else if (length == 'h') {
                        value = (unsigned short)va_arg(args, unsigned int);
                    } else {
                        value = va_arg(args, unsigned int);
                    }
                    utoa(value, buffer, 8);
                    print_text(buffer);
                    break;
                }
                case 'p': {
                    void *ptr_value = va_arg(args, void*);
                    uint64_t addr = (uint64_t)ptr_value;
                    utoa(addr, buffer, 16);
                    print_text("0x");
                    print_text(buffer);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    char str[2] = {c, '\0'};
                    print_text(str);
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char*);
                    print_text(str);
                    break;
                }
                case '%': {
                    print_text("%");
                    break;
                }
                default:
                    // If the format is invalid, print it directly
                    print_text("%");
                    char invalid[2] = {*fmt_ptr, '\0'};
                    print_text(invalid);
                    break;
            }
        } else {
            // Print regular characters
            char ch[2] = {*fmt_ptr, '\0'};
            print_text(ch);
        }
    }

    va_end(args);
}

void put_char(char c) {
    // Xử lý ký tự xuống dòng
    if (c == '\n') {
        g_ctx.cursor_x = 0;  // Đặt con trỏ ngang về đầu dòng
        g_ctx.cursor_y += g_ctx.line_height;  // Di chuyển con trỏ xuống dòng, sử dụng chiều cao dòng từ font
        if (g_ctx.cursor_y + g_ctx.line_height > g_ctx.height) {
            scroll_screen();  // Hàm này cần phải cuộn màn hình
            g_ctx.cursor_y -= g_ctx.line_height;  // Điều chỉnh lại con trỏ sau khi cuộn
        }
        return;
    }

    // Đảm bảo ký tự nằm trong khoảng ký tự có thể in được (32-126)
    if (c < 32 || c > 126) {
        c = '?';  // Ký tự ngoài khoảng, dùng '?' làm mặc định
    }

    // Vẽ ký tự dựa trên glyph
    draw_glyph(g_ctx.cursor_x, g_ctx.cursor_y, c);

    // Lấy thông tin glyph để biết khoảng cách con trỏ cần di chuyển
    const glyph_t *glyph = &roboto_glyphs[c - 32];  // Lấy glyph của ký tự
    g_ctx.cursor_x += glyph->x_advance;  // Di chuyển con trỏ ngang dựa trên glyph

    // Nếu con trỏ vượt quá chiều rộng màn hình, xuống dòng
    if (g_ctx.cursor_x + glyph->width > g_ctx.width) {
        g_ctx.cursor_x = 0;  // Đặt con trỏ ngang về đầu dòng
        g_ctx.cursor_y += g_ctx.line_height;  // Xuống dòng
        if (g_ctx.cursor_y + g_ctx.line_height > g_ctx.height) {
            scroll_screen();  // Hàm này cần phải cuộn màn hình
            g_ctx.cursor_y -= g_ctx.line_height;  // Điều chỉnh lại con trỏ sau khi cuộn
        }
    }
}
