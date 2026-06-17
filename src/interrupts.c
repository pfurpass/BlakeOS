#include "interrupts.h"
#include "serial.h"
#include "pic.h"

// Klartextnamen der CPU-Ausnahmen – hilft beim Debuggen enorm.
static const char *exception_names[32] = {
    "Division durch Null", "Debug", "NMI", "Breakpoint",
    "Overflow", "Bound Range", "Ungueltiger Opcode", "Kein Coprozessor",
    "Double Fault", "Coprozessor-Segment", "Ungueltiges TSS", "Segment fehlt",
    "Stack-Fault", "General Protection Fault", "Page Fault", "reserviert",
    "x87-FP-Fehler", "Alignment Check", "Machine Check", "SIMD-FP-Fehler",
    "Virtualisierung", "Control Protection", "reserviert", "reserviert",
    "reserviert", "reserviert", "reserviert", "reserviert",
    "Hypervisor Injection", "VMM Communication", "Security", "reserviert"
};

// Tabelle der registrierten IRQ-Handler (IRQ 0–15).
static irq_handler_t irq_handlers[16] = { 0 };

void register_irq_handler(uint8_t irq, irq_handler_t handler) {
    if (irq < 16) irq_handlers[irq] = handler;
}

// Wird aus isr_common (isr.S) aufgerufen.
void isr_handler(registers_t *regs) {
    uint64_t n = regs->int_no;

    if (n < 32) {
        // CPU-Ausnahme: melden und anhalten.
        serial_write("\n[AUSNAHME] ");
        serial_write(exception_names[n]);
        serial_write(" (vector=");
        serial_write_hex(n);
        serial_write(", err=");
        serial_write_hex(regs->err_code);
        serial_write(", rip=");
        serial_write_hex(regs->rip);
        serial_write(")\nSystem angehalten.\n");
        for (;;) __asm__ volatile ("cli; hlt");
    }

    if (n >= 32 && n < 48) {
        uint8_t irq = (uint8_t)(n - 32);
        if (irq_handlers[irq]) {
            irq_handlers[irq](regs);
        }
        // Dem PIC mitteilen: Interrupt fertig bearbeitet.
        pic_send_eoi(irq);
    }
}
