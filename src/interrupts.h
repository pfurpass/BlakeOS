#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

// Gesicherte Register beim Interrupt. Die Reihenfolge MUSS exakt zur
// Push-Reihenfolge in isr.S passen (niedrigste Adresse zuerst).
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;          // von den Stubs gepusht
    uint64_t rip, cs, rflags, rsp, ss;  // von der CPU gepusht
} registers_t;

typedef void (*irq_handler_t)(registers_t *regs);

// Registriert eine Funktion für eine Hardware-IRQ-Nummer (0–15).
void register_irq_handler(uint8_t irq, irq_handler_t handler);

#endif
