#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

// Initialisiert COM1 (38400 Baud, 8N1). Gibt 0 zurück bei Erfolg,
// !=0 wenn der Selbsttest des UART fehlschlägt.
int serial_init(void);

void serial_putchar(char c);
void serial_write(const char *s);

// Gibt einen 64-Bit-Wert als Hex aus (z. B. 0x00000000DEADBEEF) – fürs Debuggen.
void serial_write_hex(uint64_t value);

#endif
