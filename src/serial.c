#include "serial.h"
#include "io.h"

// COM1-Basisadresse und Registeroffsets des 16550-UART.
#define COM1            0x3F8
#define REG_DATA        (COM1 + 0)  // Daten / Divisor low (bei DLAB=1)
#define REG_INT_ENABLE  (COM1 + 1)  // Interrupts / Divisor high
#define REG_FIFO_CTRL   (COM1 + 2)  // FIFO-Steuerung
#define REG_LINE_CTRL   (COM1 + 3)  // Leitungssteuerung (DLAB-Bit)
#define REG_MODEM_CTRL  (COM1 + 4)  // Modemsteuerung
#define REG_LINE_STATUS (COM1 + 5)  // Leitungsstatus

int serial_init(void) {
    outb(REG_INT_ENABLE, 0x00); // Interrupts aus
    outb(REG_LINE_CTRL,  0x80); // DLAB an: nächste 2 Schreibzugriffe = Baudteiler
    outb(REG_DATA,       0x03); // Teiler low  = 3  -> 115200/3 = 38400 Baud
    outb(REG_INT_ENABLE, 0x00); // Teiler high = 0
    outb(REG_LINE_CTRL,  0x03); // DLAB aus, 8 Bit, keine Parität, 1 Stoppbit
    outb(REG_FIFO_CTRL,  0xC7); // FIFO an, leeren, Schwelle 14 Byte
    outb(REG_MODEM_CTRL, 0x0B); // RTS/DSR gesetzt, OUT2 an

    // Selbsttest: UART in Loopback-Modus, Testbyte senden, zurücklesen.
    outb(REG_MODEM_CTRL, 0x1E);
    outb(REG_DATA, 0xAE);
    if (inb(REG_DATA) != 0xAE) {
        return 1; // UART defekt / nicht vorhanden
    }

    // Zurück in den normalen Betriebsmodus.
    outb(REG_MODEM_CTRL, 0x0F);
    return 0;
}

// Wartet, bis das Sende-Halteregister leer ist (Bit 5 im Leitungsstatus).
static int transmit_empty(void) {
    return inb(REG_LINE_STATUS) & 0x20;
}

void serial_putchar(char c) {
    if (c == '\n') {           // Terminals erwarten CR+LF
        while (!transmit_empty());
        outb(REG_DATA, '\r');
    }
    while (!transmit_empty());
    outb(REG_DATA, (uint8_t)c);
}

void serial_write(const char *s) {
    for (; *s; s++) serial_putchar(*s);
}

void serial_write_hex(uint64_t value) {
    const char *digits = "0123456789ABCDEF";
    serial_putchar('0');
    serial_putchar('x');
    // 16 Hex-Ziffern, höchstwertige zuerst.
    for (int shift = 60; shift >= 0; shift -= 4) {
        serial_putchar(digits[(value >> shift) & 0xF]);
    }
}
