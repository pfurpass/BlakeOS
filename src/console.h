#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

// Initialisiert die Konsole. 'scale' vergrößert die 8x8-Glyphen
// (scale=2 -> 16x16 Pixel pro Zeichen) für bessere Lesbarkeit.
void console_init(uint32_t fg, uint32_t bg, uint32_t scale);

void console_set_color(uint32_t fg, uint32_t bg);
void console_set_cursor(uint64_t col, uint64_t row);

void console_putchar(char c);
void console_write(const char *s);

#endif
