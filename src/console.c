#include "console.h"
#include "framebuffer.h"
#include "font.h"

static uint32_t c_fg;       // Vordergrundfarbe
static uint32_t c_bg;       // Hintergrundfarbe
static uint32_t c_scale;    // Vergrößerungsfaktor
static uint64_t cur_col;    // Cursor-Spalte (in Zeichen)
static uint64_t cur_row;    // Cursor-Zeile (in Zeichen)

void console_init(uint32_t fg, uint32_t bg, uint32_t scale) {
    c_fg = fg;
    c_bg = bg;
    c_scale = (scale == 0) ? 1 : scale;
    cur_col = 0;
    cur_row = 0;
}

void console_set_color(uint32_t fg, uint32_t bg) {
    c_fg = fg;
    c_bg = bg;
}

void console_set_cursor(uint64_t col, uint64_t row) {
    cur_col = col;
    cur_row = row;
}

// Zeichnet ein einzelnes Glyph an der Pixelposition (px, py).
static void draw_glyph(char c, uint64_t px, uint64_t py) {
    const uint8_t *glyph = font_glyph(c);
    for (uint32_t row = 0; row < FONT_HEIGHT; row++) {
        for (uint32_t col = 0; col < FONT_WIDTH; col++) {
            uint32_t color = (glyph[row] & (1 << col)) ? c_fg : c_bg;
            // Jedes Font-Pixel als scale x scale Block zeichnen.
            fb_fill_rect(px + col * c_scale, py + row * c_scale,
                         c_scale, c_scale, color);
        }
    }
}

static void newline(void) {
    cur_col = 0;
    cur_row++;
}

void console_putchar(char c) {
    if (c == '\n') { newline(); return; }
    if (c == '\r') { cur_col = 0; return; }

    uint64_t glyph_w = FONT_WIDTH  * c_scale;
    uint64_t glyph_h = FONT_HEIGHT * c_scale;

    // Zeilenumbruch am rechten Rand.
    if ((cur_col + 1) * glyph_w > fb.width) newline();

    draw_glyph(c, cur_col * glyph_w, cur_row * glyph_h);
    cur_col++;
}

void console_write(const char *s) {
    for (; *s; s++) console_putchar(*s);
}
