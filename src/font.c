#include "font.h"
#include "font8x8_basic.h"  // definiert: char font8x8_basic[128][8]

const uint8_t *font_glyph(char c) {
    unsigned char uc = (unsigned char)c;
    if (uc >= 128) uc = '?';            // außerhalb des Bereichs -> Platzhalter
    return (const uint8_t *)font8x8_basic[uc];
}
