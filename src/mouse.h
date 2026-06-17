#ifndef MOUSE_H
#define MOUSE_H

// Initialisiert die PS/2-Maus, registriert den Handler auf IRQ12 und
// schaltet die nötigen Interrupt-Leitungen frei.
void mouse_init(void);

// Aktuelle Cursorposition und Tastenzustand.
int mouse_get_x(void);
int mouse_get_y(void);
int mouse_left(void);   // 1 = linke Taste gedrückt
unsigned mouse_get_packets(void);  // Anzahl empfangener Pakete (Diagnose)

#endif
