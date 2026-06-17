#include "desktop.h"
#include "framebuffer.h"
#include "console.h"
#include "mouse.h"

#define TITLE_H 30
#define TASKBAR_H 30

// Ein einziges Fenster (Position/Größe/Titel).
static int win_x = 140, win_y = 110, win_w = 460, win_h = 270;
static const char *win_title = "Willkommen";

// Zustand fürs Ziehen.
static int dragging = 0;
static int drag_off_x = 0, drag_off_y = 0;
static int prev_left = 0;

// Mauszeiger als kleine Bitmap. 'X' = schwarzer Rand, '*' = weiße Füllung,
// ' ' = durchsichtig. Die Spitze sitzt oben links auf der Mausposition.
static const char *cursor[] = {
    "X          ",
    "XX         ",
    "X*X        ",
    "X**X       ",
    "X***X      ",
    "X****X     ",
    "X*****X    ",
    "X******X   ",
    "X*******X  ",
    "X********X ",
    "X*********X",
    "X*****XXXXX",
    "X**X**X    ",
    "X*X X**X   ",
    "XX  X**X   ",
    "X    X**X  ",
    "     X**X  ",
    "      XX   ",
};

// Hängt eine Zahl als Dezimaltext an einen Puffer an (kleiner Helfer, da wir
// noch keine Standardbibliothek haben).
static void append_uint(char *buf, int *pos, unsigned v) {
    char tmp[12];
    int n = 0;
    if (v == 0) tmp[n++] = '0';
    while (v) { tmp[n++] = (char)('0' + v % 10); v /= 10; }
    while (n) buf[(*pos)++] = tmp[--n];
}

static void draw_outline(int x, int y, int w, int h, uint32_t color) {
    fb_fill_rect(x, y, w, 1, color);              // oben
    fb_fill_rect(x, y + h - 1, w, 1, color);      // unten
    fb_fill_rect(x, y, 1, h, color);              // links
    fb_fill_rect(x + w - 1, y, 1, h, color);      // rechts
}

static void draw_cursor(int mx, int my) {
    const int rows = (int)(sizeof(cursor) / sizeof(cursor[0]));
    const int s = 2; // Vergrößerung des Zeigers
    for (int row = 0; row < rows; row++) {
        const char *line = cursor[row];
        for (int col = 0; line[col]; col++) {
            uint32_t c;
            if (line[col] == 'X')      c = rgb(0x00, 0x00, 0x00);
            else if (line[col] == '*') c = rgb(0xFF, 0xFF, 0xFF);
            else continue;
            fb_fill_rect(mx + col * s, my + row * s, s, s, c);
        }
    }
}

void desktop_update(void) {
    int mx = mouse_get_x();
    int my = mouse_get_y();
    int left = mouse_left();

    // Linke Taste neu gedrückt und Cursor auf der Titelleiste? -> Ziehen starten.
    if (left && !prev_left) {
        if (mx >= win_x && mx < win_x + win_w &&
            my >= win_y && my < win_y + TITLE_H) {
            dragging = 1;
            drag_off_x = mx - win_x;
            drag_off_y = my - win_y;
        }
    }
    if (!left) dragging = 0;

    if (dragging) {
        win_x = mx - drag_off_x;
        win_y = my - drag_off_y;
    }
    prev_left = left;
}

void desktop_render(void) {
    // Hintergrund (Wallpaper).
    fb_clear(rgb(0x10, 0x14, 0x20));

    // Leichter Schatten hinter dem Fenster.
    fb_fill_rect(win_x + 6, win_y + 6, win_w, win_h, rgb(0x08, 0x0A, 0x10));

    // Fensterkörper.
    fb_fill_rect(win_x, win_y, win_w, win_h, rgb(0x1E, 0x24, 0x32));

    // Titelleiste – heller, wenn gerade gezogen wird.
    uint32_t title_col = dragging ? rgb(0x5A, 0x9B, 0xF0) : rgb(0x3A, 0x6E, 0xA8);
    fb_fill_rect(win_x, win_y, win_w, TITLE_H, title_col);

    // Rahmen.
    draw_outline(win_x, win_y, win_w, win_h, rgb(0x5A, 0x9B, 0xF0));

    // Titel- und Fließtext.
    console_draw_text(win_x + 10, win_y + 8, 2, rgb(0xFF, 0xFF, 0xFF), win_title);
    console_draw_text(win_x + 16, win_y + TITLE_H + 22, 2,
                      rgb(0xC0, 0xC8, 0xD4),
                      "Zieh mich an der\nTitelleiste herum!");

    // Taskleiste am unteren Rand.
    int ty = (int)fb.height - TASKBAR_H;
    fb_fill_rect(0, ty, fb.width, TASKBAR_H, rgb(0x24, 0x2A, 0x3A));
    console_draw_text(12, ty + 8, 2, rgb(0xF0, 0xF0, 0xF0), "BlakeOS");

    // Diagnose: Mausposition und Paketzähler rechts in der Taskleiste.
    char status[48];
    int p = 0;
    const char *lbl = "Maus X=";
    for (const char *q = lbl; *q; q++) status[p++] = *q;
    append_uint(status, &p, (unsigned)mouse_get_x());
    status[p++] = ' '; status[p++] = 'Y'; status[p++] = '=';
    append_uint(status, &p, (unsigned)mouse_get_y());
    status[p++] = ' '; status[p++] = 'P'; status[p++] = '=';
    append_uint(status, &p, mouse_get_packets());
    status[p] = 0;
    console_draw_text((int)fb.width - 380, ty + 8, 2,
                      rgb(0xC0, 0xC8, 0xD4), status);

    // Mauszeiger immer ganz oben.
    draw_cursor(mouse_get_x(), mouse_get_y());
}
