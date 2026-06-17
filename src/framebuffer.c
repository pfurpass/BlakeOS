#include "framebuffer.h"

framebuffer_t fb;

// Back-Buffer: wir zeichnen alles hier hinein und kopieren ihn dann mit
// fb_present() in einem Rutsch auf den Bildschirm. Das verhindert Flackern.
// Dimensioniert für bis zu 1920x1080 (~8 MB, liegt im .bss).
#define FB_MAX_PIXELS (1920u * 1080u)
static uint32_t backbuffer[FB_MAX_PIXELS];

void fb_init(void *address, uint64_t width, uint64_t height, uint64_t pitch) {
    fb.pixels = (volatile uint32_t *)address;
    fb.width  = width;
    fb.height = height;
    fb.pitch  = pitch;
}

void fb_put_pixel(uint64_t x, uint64_t y, uint32_t color) {
    if (x >= fb.width || y >= fb.height) return;
    uint64_t idx = y * fb.width + x;          // Back-Buffer ist dicht gepackt
    if (idx >= FB_MAX_PIXELS) return;         // Sicherheitsgrenze
    backbuffer[idx] = color;
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

// Back-Buffer auf den echten Bildschirm kopieren (Zeile für Zeile, da die
// Hardware-Zeile pitch Bytes breit ist, der Back-Buffer aber width Pixel).
void fb_present(void) {
    uint32_t hw_stride = (uint32_t)(fb.pitch / 4);
    for (uint64_t y = 0; y < fb.height; y++) {
        volatile uint32_t *dst = fb.pixels + y * hw_stride;
        uint32_t *src = backbuffer + y * fb.width;
        if ((y * fb.width + fb.width) > FB_MAX_PIXELS) break;
        for (uint64_t x = 0; x < fb.width; x++) {
            dst[x] = src[x];
        }
    }
}
