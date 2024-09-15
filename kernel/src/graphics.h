#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <stddef.h>

// Định nghĩa cấu trúc ngữ cảnh đồ họa
typedef struct {
    uint32_t *framebuffer;
    size_t pitch;
    size_t width;
    size_t height;
    uint32_t text_color;
    uint32_t background_color;
    int cursor_x;
    int cursor_y;
    int line_height;
    int max_rows;
    // Thêm biến lưu vị trí con trỏ cũ
    int prev_cursor_x;
    int prev_cursor_y;
} graphics_context_t;

// Khai báo các hàm đồ họa
void put_pixel(graphics_context_t *ctx, int x, int y, uint32_t color);
void draw_glyph(graphics_context_t *ctx, int x, int y, char c);
void draw_text(graphics_context_t *ctx, int x, int y, const char *text);
void print_text(graphics_context_t *ctx, const char *text);
void scroll_screen(graphics_context_t *ctx);
void draw_cursor(graphics_context_t *ctx);
void erase_cursor(graphics_context_t *ctx);

#endif // GRAPHICS_H
