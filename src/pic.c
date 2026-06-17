#include "pic.h"
#include "io.h"

#define PIC1_CMD   0x20
#define PIC1_DATA  0x21
#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1
#define PIC_EOI    0x20

void pic_remap(uint8_t offset1, uint8_t offset2) {
    // Initialisierungssequenz (ICW1–ICW4).
    outb(PIC1_CMD, 0x11); io_wait();   // ICW1: Initialisierung, ICW4 folgt
    outb(PIC2_CMD, 0x11); io_wait();
    outb(PIC1_DATA, offset1); io_wait(); // ICW2: Vektor-Offset Master
    outb(PIC2_DATA, offset2); io_wait(); // ICW2: Vektor-Offset Slave
    outb(PIC1_DATA, 0x04); io_wait();    // ICW3: Slave hängt an IRQ2
    outb(PIC2_DATA, 0x02); io_wait();    // ICW3: Slave-Identität
    outb(PIC1_DATA, 0x01); io_wait();    // ICW4: 8086-Modus
    outb(PIC2_DATA, 0x01); io_wait();

    // Vorerst alle IRQs sperren; einzelne werden gezielt freigeschaltet.
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) outb(PIC2_CMD, PIC_EOI); // Slave zuerst bestätigen
    outb(PIC1_CMD, PIC_EOI);
}

void pic_clear_mask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    if (irq >= 8) irq -= 8;
    uint8_t value = inb(port) & ~(1 << irq);
    outb(port, value);
}

void pic_set_mask(uint8_t irq) {
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    if (irq >= 8) irq -= 8;
    uint8_t value = inb(port) | (1 << irq);
    outb(port, value);
}
