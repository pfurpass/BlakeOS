#include "keyboard.h"
#include "interrupts.h"
#include "pic.h"
#include "io.h"
#include "console.h"
#include "serial.h"

#define KBD_DATA_PORT 0x60

// Scancode-Set 1, US-Layout: ohne Shift. 0 = keine druckbare Taste.
static const char kbd_lower[128] = {
    0,   27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0,  ' '
};

// Dasselbe mit gedrückter Shift-Taste.
static const char kbd_upper[128] = {
    0,   27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,   'A','S','D','F','G','H','J','K','L',':','"','~',
    0,   '|','Z','X','C','V','B','N','M','<','>','?', 0,
    '*', 0,  ' '
};

static int shift_down = 0;

// Wird bei jedem Tastatur-IRQ aufgerufen.
static void keyboard_handler(registers_t *regs) {
    (void)regs;
    uint8_t sc = inb(KBD_DATA_PORT);

    // Shift gedrückt / losgelassen.
    if (sc == 0x2A || sc == 0x36) { shift_down = 1; return; }
    if (sc == 0xAA || sc == 0xB6) { shift_down = 0; return; }

    // Bit 7 gesetzt = Taste losgelassen -> ignorieren.
    if (sc & 0x80) return;
    if (sc >= 128) return;

    char c = shift_down ? kbd_upper[sc] : kbd_lower[sc];
    if (c) {
        console_putchar(c);   // auf den Bildschirm
        serial_putchar(c);    // und ins Debug-Log
    }
}

void keyboard_init(void) {
    register_irq_handler(1, keyboard_handler);
    pic_clear_mask(1); // IRQ1 (Tastatur) freischalten
}
