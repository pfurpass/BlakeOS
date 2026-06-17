#ifndef PIC_H
#define PIC_H

#include <stdint.h>

// Programmiert die beiden 8259-PICs neu, sodass ihre IRQs auf die
// Interrupt-Vektoren offset1 (Master) bzw. offset2 (Slave) abgebildet werden.
// Üblich: 0x20 und 0x28, damit IRQ0–15 zu Vektor 32–47 werden.
void pic_remap(uint8_t offset1, uint8_t offset2);

// Bestätigt das Ende eines Interrupts (End Of Interrupt).
void pic_send_eoi(uint8_t irq);

// Einzelne IRQ-Leitung freischalten bzw. sperren.
void pic_clear_mask(uint8_t irq);
void pic_set_mask(uint8_t irq);

#endif
