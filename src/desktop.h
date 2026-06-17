#ifndef DESKTOP_H
#define DESKTOP_H

// Verarbeitet Mauseingaben (Fenster ziehen) – einmal pro Frame aufrufen.
void desktop_update(void);

// Zeichnet den kompletten Desktop in den Back-Buffer.
void desktop_render(void);

#endif
