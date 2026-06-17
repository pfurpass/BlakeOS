#include "framebuffer.h"

framebuffer_t fb;

void fb_init(void *address, uint64_t width, uint64_t height, uint64_t pitch) {
    fb.pixels = (volatile uint32_t *)address;
    fb.width  = width;
    fb.height = height;
    fb.pitch  = pitch;
}

void fb_put_pixel(uint64_t x, uint64_t y, uint32_t color) {
    if (x >= fb.width || y >= fb.height) return;
    // pitch ist in Bytes; eine Zeile umfasst pitch/4 uint32_t-Pixel.
    fb.pixels[y * (fb.pitch / 4) + x] = color;
}

void fb_clear(uint32_t color) {
    fb_fill_rect(0, 0, fb.width, fb.height, color);
}

void fb_fill_rect(uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint32_t color) {
    for (uint64_t dy = 0; dy < h; dy++) {
        for (uint64_t dx = 0; dx < w; dx++) {
            fb_put_pixel(x + dx, y + dy, color);
        }
    }
}
