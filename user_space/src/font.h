#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define FONT_ASCENT 24  // Thay bằng giá trị thực tế từ script Python
#define FONT_DESCENT 8  // Thay bằng giá trị thực tế từ script Python
#define FONT_LINE_HEIGHT (FONT_ASCENT + FONT_DESCENT)

typedef struct {
    uint8_t width;
    uint8_t height;
    int8_t x_offset;
    int8_t y_offset;
    int8_t x_advance;
    const uint8_t *data;
} glyph_t;

extern const glyph_t roboto_glyphs[];  // Mảng chứa các glyph

#endif // FONT_H
