#include <stdint.h>
#include <stddef.h>
#include "limine.h"

#include "framebuffer.h"
#include "console.h"

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

    // Haben wir wirklich einen Framebuffer bekommen?
    if (framebuffer_request.response == NULL
        || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *lfb =
        framebuffer_request.response->framebuffers[0];

    // Unseren eigenen, vom Limine-Protokoll entkoppelten Framebuffer aufsetzen.
    fb_init(lfb->address, lfb->width, lfb->height, lfb->pitch);

    // Hintergrund: dunkles Blaugrau.
    uint32_t bg = rgb(0x14, 0x18, 0x24);
    fb_clear(bg);

    // Ein kleiner Farbbalken oben, damit man sieht: Grafik funktioniert.
    uint32_t bar_h = lfb->height / 40;
    fb_fill_rect(0, 0, lfb->width / 3,     bar_h, rgb(0xE0, 0x4F, 0x5F)); // rot
    fb_fill_rect(lfb->width / 3, 0, lfb->width / 3, bar_h, rgb(0x4F, 0xC0, 0x7F)); // grün
    fb_fill_rect(2 * lfb->width / 3, 0, lfb->width / 3, bar_h, rgb(0x4F, 0x8F, 0xE0)); // blau

    // Text rendern. scale=3 -> große, gut lesbare 24x24-Pixel-Glyphen.
    console_init(rgb(0xF0, 0xF0, 0xF0), bg, 3);
    console_set_cursor(2, 3);
    console_write("Hello, World!\n");

    console_set_color(rgb(0x8F, 0xB8, 0xFF), bg);
    console_set_cursor(2, 5);
    console_write("BlakeOS laeuft.\n");

    console_set_color(rgb(0x9A, 0x9A, 0x9A), bg);
    console_set_cursor(2, 7);
    console_write("Framebuffer aktiv - bereit zum Erweitern.\n");

    // Kernel ist fertig: anhalten (kein Scheduler, keine Interrupts ... noch).
    hcf();
}
