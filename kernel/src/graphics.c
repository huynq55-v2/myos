#include "graphics.h"
#include "font.h"  // Để truy cập dữ liệu font như `roboto_glyphs`
#include <limine.h>  // Nếu cần sử dụng `struct limine_framebuffer`
#include <klibc.h>

void put_pixel(graphics_context_t *ctx, int x, int y, uint32_t color) {
    if (x >= 0 && x < ctx->width && y >= 0 && y < ctx->height) {
        ctx->framebuffer[y * (ctx->pitch / 4) + x] = color;
    }
}

void draw_glyph(graphics_context_t *ctx, int x, int y, char c) {
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
                put_pixel(ctx, px, py, ctx->text_color);
            }
        }
    }
}

void draw_text(graphics_context_t *ctx, int x, int y, const char *text) {
    ctx->cursor_x = x;
    ctx->cursor_y = y;

    while (*text) {
        if (*text == '\n') {
            ctx->cursor_x = x;
            ctx->cursor_y += ctx->line_height;
        } else {
            draw_glyph(ctx, ctx->cursor_x, ctx->cursor_y, *text);
            const glyph_t *glyph = &roboto_glyphs[*text - 32];
            ctx->cursor_x += glyph->x_advance;
        }
        text++;
    }
}

void print_text(graphics_context_t *ctx, const char *text) {
    // Xóa dấu nháy con trỏ cũ
    erase_cursor(ctx);

    while (*text) {
        if (*text == '\n') {
            // Di chuyển con trỏ đến đầu dòng mới
            ctx->cursor_x = 0;
            ctx->cursor_y += ctx->line_height;

            // Kiểm tra nếu cần cuộn màn hình
            if (ctx->cursor_y + ctx->line_height > ctx->height) {
                scroll_screen(ctx);
                ctx->cursor_y -= ctx->line_height;
            }
        } else {
            // Vẽ ký tự
            draw_glyph(ctx, ctx->cursor_x, ctx->cursor_y, *text);

            // Lấy glyph để biết x_advance
            const glyph_t *glyph = &roboto_glyphs[*text - 32];
            ctx->cursor_x += glyph->x_advance;

            // Kiểm tra nếu con trỏ vượt quá chiều rộng màn hình
            if (ctx->cursor_x + glyph->width > ctx->width) {
                // Di chuyển con trỏ đến đầu dòng mới
                ctx->cursor_x = 0;
                ctx->cursor_y += ctx->line_height;

                // Kiểm tra nếu cần cuộn màn hình
                if (ctx->cursor_y + ctx->line_height > ctx->height) {
                    scroll_screen(ctx);
                    ctx->cursor_y -= ctx->line_height;
                }
            }
        }
        text++;
    }

    // Vẽ dấu nháy con trỏ mới
    draw_cursor(ctx);
}

void scroll_screen(graphics_context_t *ctx) {
    // Tính số byte mỗi dòng
    size_t row_size = ctx->pitch * ctx->line_height;

    // Tính số byte cần di chuyển (từ dòng thứ hai đến dòng cuối)
    size_t move_size = ctx->pitch * (ctx->height - ctx->line_height);

    // Di chuyển vùng nhớ lên trên một dòng
    memmove(
        ctx->framebuffer,
        (uint8_t *)ctx->framebuffer + row_size,
        move_size
    );

    // Xóa dòng cuối cùng
    size_t last_row_offset = ctx->pitch * (ctx->height - ctx->line_height) / 4;
    for (size_t y = 0; y < ctx->line_height; y++) {
        for (size_t x = 0; x < ctx->width; x++) {
            ctx->framebuffer[last_row_offset + y * (ctx->pitch / 4) + x] = ctx->background_color;
        }
    }
}

void erase_cursor(graphics_context_t *ctx) {
    int cursor_width = 2;
    int cursor_height = ctx->line_height;

    for (int y = 0; y < cursor_height; y++) {
        for (int x = 0; x < cursor_width; x++) {
            int px = ctx->prev_cursor_x + x;
            int py = ctx->prev_cursor_y + y;
            if (px >= 0 && px < ctx->width && py >= 0 && py < ctx->height) {
                ctx->framebuffer[py * (ctx->pitch / 4) + px] = ctx->background_color;
            }
        }
    }
}

void draw_cursor(graphics_context_t *ctx) {
    int cursor_width = 2;
    int cursor_height = ctx->line_height;

    // Cập nhật vị trí con trỏ cũ trước khi vẽ dấu nháy mới
    ctx->prev_cursor_x = ctx->cursor_x;
    ctx->prev_cursor_y = ctx->cursor_y;

    for (int y = 0; y < cursor_height; y++) {
        for (int x = 0; x < cursor_width; x++) {
            int px = ctx->cursor_x + x;
            int py = ctx->cursor_y + y;
            if (px >= 0 && px < ctx->width && py >= 0 && py < ctx->height) {
                ctx->framebuffer[py * (ctx->pitch / 4) + px] = ctx->text_color;
            }
        }
    }
}

