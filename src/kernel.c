#include <stdint.h>
#include <stddef.h>
#include "limine.h"

#include "framebuffer.h"
#include "console.h"
#include "serial.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "mouse.h"
#include "desktop.h"

// --- Limine-Protokoll: Anfragen an den Bootloader ---------------------------
// Diese Strukturen liest Limine aus dem Kernel-Image, bevor es uns startet.

// Wir verlangen Basis-Revision 2 des Protokolls.
__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(2);

// Wir bitten um einen linearen Grafik-Framebuffer.
__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

// Start-/End-Marker, damit der Linker die Anfragen sauber platziert.
__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

// --- Hilfsfunktion: anhalten -------------------------------------------------
static void hcf(void) {
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

// --- Kernel-Einstiegspunkt ---------------------------------------------------
// Limine springt direkt hierher (siehe ENTRY(kmain) im Linker-Skript).
void kmain(void) {
    // Prüfen, ob der Bootloader unsere Protokoll-Revision unterstützt.
    if (!LIMINE_BASE_REVISION_SUPPORTED) {
        hcf();
    }

    // Serielle Konsole zuerst: ab jetzt können wir loggen, selbst wenn
    // beim Framebuffer etwas schiefgeht.
    serial_init();
    serial_write("\n=== BlakeOS startet ===\n");

    // Haben wir wirklich einen Framebuffer bekommen?
    if (framebuffer_request.response == NULL
        || framebuffer_request.response->framebuffer_count < 1) {
        serial_write("FEHLER: kein Framebuffer vom Bootloader.\n");
        hcf();
    }

    struct limine_framebuffer *lfb =
        framebuffer_request.response->framebuffers[0];

    // Debug-Infos über die serielle Leitung ausgeben.
    serial_write("Framebuffer-Adresse: ");
    serial_write_hex((uint64_t)lfb->address);
    serial_write("\nBreite:  ");
    serial_write_hex(lfb->width);
    serial_write("\nHoehe:   ");
    serial_write_hex(lfb->height);
    serial_write("\nPitch:   ");
    serial_write_hex(lfb->pitch);
    serial_putchar('\n');

    // Unseren eigenen, vom Limine-Protokoll entkoppelten Framebuffer aufsetzen.
    fb_init(lfb->address, lfb->width, lfb->height, lfb->pitch);

    // --- Interrupts, Tastatur und Maus einrichten ---
    pic_remap(0x20, 0x28);   // IRQ0–15 -> Vektor 32–47
    idt_init();              // Interrupt-Tabelle laden
    keyboard_init();         // Tastatur-Handler auf IRQ1
    mouse_init();            // Maus-Handler auf IRQ12
    __asm__ volatile ("sti"); // Interrupts scharfschalten

    serial_write("Desktop aktiv. Warte auf Maus/Tastatur.\n");

    // Haupt-Schleife: Eingaben verarbeiten, Desktop zeichnen, auf den
    // Bildschirm bringen, dann bis zum nächsten Interrupt schlafen.
    for (;;) {
        desktop_update();
        desktop_render();
        fb_present();
        __asm__ volatile ("hlt");
    }
}
