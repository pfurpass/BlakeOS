#include "mouse.h"
#include "interrupts.h"
#include "pic.h"
#include "io.h"
#include "framebuffer.h"
#include "serial.h"

#define PS2_DATA   0x60
#define PS2_STATUS 0x64
#define PS2_CMD    0x64

static int mouse_x = 0;
static int mouse_y = 0;
static int buttons = 0;
static volatile unsigned packet_count = 0;  // wie viele Pakete kamen an?

int mouse_get_x(void) { return mouse_x; }
int mouse_get_y(void) { return mouse_y; }
int mouse_left(void)  { return buttons & 0x01; }
unsigned mouse_get_packets(void) { return packet_count; }

// Warten, bis man schreiben darf (Eingabepuffer leer, Statusbit 1 = 0).
static void wait_write(void) {
    for (int i = 0; i < 100000; i++)
        if (!(inb(PS2_STATUS) & 0x02)) return;
}
// Warten, bis Daten bereitstehen (Ausgabepuffer voll, Statusbit 0 = 1).
static void wait_read(void) {
    for (int i = 0; i < 100000; i++)
        if (inb(PS2_STATUS) & 0x01) return;
}

// Ein Byte an die Maus (zweiter Port) senden.
static void mouse_command(uint8_t value) {
    wait_write(); outb(PS2_CMD, 0xD4);   // "nächstes Byte geht an die Maus"
    wait_write(); outb(PS2_DATA, value);
}
static uint8_t mouse_response(void) {
    wait_read(); return inb(PS2_DATA);   // i. d. R. 0xFA (ACK)
}

// Zusammenbau der 3-Byte-Pakete.
static uint8_t packet[3];
static uint8_t cycle = 0;

static void mouse_handler(registers_t *regs) {
    (void)regs;
    uint8_t status = inb(PS2_STATUS);
    if (!(status & 0x01)) return;        // keine Daten
    if (!(status & 0x20)) {              // Bit 5: stammt vom zweiten Port (Maus)
        inb(PS2_DATA);                   // sonst verwerfen
        return;
    }
    uint8_t data = inb(PS2_DATA);

    switch (cycle) {
        case 0:
            if (!(data & 0x08)) return;  // Synchronbit fehlt -> Paket verwerfen
            packet[0] = data; cycle = 1; break;
        case 1:
            packet[1] = data; cycle = 2; break;
        case 2:
            packet[2] = data; cycle = 0;

            // Überlauf-Pakete ignorieren.
            if (packet[0] & 0xC0) break;

            int dx = (int)(int8_t)packet[1];
            int dy = (int)(int8_t)packet[2];
            mouse_x += dx;
            mouse_y -= dy;               // Y ist invertiert (PS/2: oben = positiv)
            buttons = packet[0] & 0x07;

            // Auf den Bildschirm begrenzen.
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x >= (int)fb.width)  mouse_x = (int)fb.width  - 1;
            if (mouse_y >= (int)fb.height) mouse_y = (int)fb.height - 1;

            // Die ersten Pakete zum Debuggen ins Serien-Log schreiben.
            if (packet_count < 4) {
                serial_write("Maus-Paket: ");
                serial_write_hex(packet[0]); serial_putchar(' ');
                serial_write_hex(packet[1]); serial_putchar(' ');
                serial_write_hex(packet[2]); serial_putchar('\n');
            }
            packet_count++;
            break;
    }
}

void mouse_init(void) {
    mouse_x = (int)fb.width  / 2;
    mouse_y = (int)fb.height / 2;

    // Eventuell wartende Bytes aus dem Ausgabepuffer verwerfen.
    for (int i = 0; i < 32 && (inb(PS2_STATUS) & 0x01); i++) inb(PS2_DATA);

    wait_write(); outb(PS2_CMD, 0xA8);   // zweiten PS/2-Port (Maus) aktivieren

    // Konfigurationsbyte lesen, IRQ12 einschalten, Maustakt aktivieren.
    wait_write(); outb(PS2_CMD, 0x20);
    uint8_t config = mouse_response();
    serial_write("Maus-Config-Byte (vorher): ");
    serial_write_hex(config); serial_putchar('\n');
    config |=  (1 << 1);                 // IRQ12 (Maus) erlauben
    config &= ~(1 << 5);                 // Maustakt nicht sperren
    wait_write(); outb(PS2_CMD, 0x60);
    wait_write(); outb(PS2_DATA, config);

    mouse_command(0xF6); mouse_response(); // Standardwerte setzen
    mouse_command(0xF4); mouse_response(); // Datenübertragung einschalten

    register_irq_handler(12, mouse_handler);
    pic_clear_mask(2);   // Kaskade: ohne IRQ2 erreichen Slave-IRQs die CPU nicht
    pic_clear_mask(12);  // IRQ12 (Maus) freischalten

    serial_write("Maus initialisiert (IRQ12).\n");
}
