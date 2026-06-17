#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <stddef.h>

// Eine vereinfachte Sicht auf den vom Bootloader bereitgestellten Bildspeicher.
// Wir kopieren nur die Felder, die wir brauchen, damit der Rest des Kernels
// nichts vom Limine-Protokoll wissen muss.
typedef struct {
    volatile uint32_t *pixels; // Basisadresse des Bildspeichers
    uint64_t width;            // Breite in Pixeln
    uint64_t height;           // Höhe in Pixeln
    uint64_t pitch;            // Bytes pro Bildzeile (kann > width*4 sein!)
} framebuffer_t;

// Globaler Framebuffer, von kernel.c initialisiert.
extern framebuffer_t fb;

// 0xRRGGBB -> packed Pixel. Limine liefert auf üblicher Hardware 32bpp.
static inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

void fb_init(void *address, uint64_t width, uint64_t height, uint64_t pitch);
void fb_clear(uint32_t color);
void fb_put_pixel(uint64_t x, uint64_t y, uint32_t color);
void fb_fill_rect(uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint32_t color);

#endif
