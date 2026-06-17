#ifndef IO_H
#define IO_H

#include <stdint.h>

// Ein Byte an einen I/O-Port schreiben.
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Ein Byte von einem I/O-Port lesen.
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Kurze Pause (ein Schreibzugriff auf einen unbenutzten Port).
// Nützlich, wenn alte Hardware Zeit zum Reagieren braucht (z. B. der PIC).
static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif
