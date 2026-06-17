#ifndef FONT_H
#define FONT_H

#include <stdint.h>

// Maße eines Glyphen in Pixeln.
#define FONT_WIDTH  8
#define FONT_HEIGHT 8

// Liefert einen Zeiger auf die 8 Bytes des Glyphen für 'c'.
// Jedes Byte ist eine Zeile; Bit n (von LSB) ist die Spalte n.
const uint8_t *font_glyph(char c);

#endif
