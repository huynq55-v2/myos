#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

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
void init_graphics(struct limine_framebuffer *fb);
void put_pixel(int x, int y, uint32_t color);
void draw_glyph(int x, int y, char c);
void draw_text(int x, int y, const char *text);
void print_text(const char *text);
void scroll_screen();
void draw_cursor();
void erase_cursor();
void print(const char *text);

#endif // GRAPHICS_H
