#include "idt.h"
#include <stdint.h>

// Ein IDT-Eintrag im 64-Bit-Modus ist 16 Bytes groß.
typedef struct __attribute__((packed)) {
    uint16_t offset_low;   // Bits 0–15 der Handler-Adresse
    uint16_t selector;     // Code-Segment-Selektor (CS)
    uint8_t  ist;          // Interrupt-Stack-Table-Index (0 = keiner)
    uint8_t  type_attr;    // Typ + Attribute (0x8E = present, Ring 0, IRQ-Gate)
    uint16_t offset_mid;   // Bits 16–31
    uint32_t offset_high;  // Bits 32–63
    uint32_t zero;         // reserviert
} idt_entry_t;

// Das Register, das wir mit 'lidt' laden.
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} idtr_t;

static idt_entry_t idt[256];
static idtr_t idtr;

// Adressen der 48 Assembler-Stubs (aus isr.S).
extern void *isr_stub_table[];

static void set_gate(int vec, void *handler, uint16_t selector, uint8_t flags) {
    uint64_t addr = (uint64_t)handler;
    idt[vec].offset_low  = addr & 0xFFFF;
    idt[vec].selector    = selector;
    idt[vec].ist         = 0;
    idt[vec].type_attr   = flags;
    idt[vec].offset_mid  = (addr >> 16) & 0xFFFF;
    idt[vec].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[vec].zero        = 0;
}

void idt_init(void) {
    // Aktuellen Code-Segment-Selektor auslesen (Limine hat ihn gesetzt),
    // statt einen festen Wert anzunehmen.
    uint64_t cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));

    for (int i = 0; i < 48; i++) {
        set_gate(i, isr_stub_table[i], (uint16_t)cs, 0x8E);
    }

    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint64_t)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idtr));
}
