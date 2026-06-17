# BlakeOS – ein minimales Hobby-Betriebssystem

Bootet über den **Limine**-Bootloader (BIOS + UEFI), bekommt einen linearen
Grafik-Framebuffer und rendert Text mit einer Bitmap-Schrift. Modular aufgebaut,
damit du Stück für Stück erweitern kannst.

![Screenshot](screenshot.png)

## Aufbau

```
blakeos/
├── Makefile            # baut Kernel + bootfähiges ISO, startet QEMU
├── linker.ld           # platziert den Kernel im higher half (0xffffffff80000000)
├── limine.conf         # Boot-Menü-Konfiguration
├── include/
│   ├── limine.h        # Limine-Boot-Protokoll (vom Bootloader)
│   └── font8x8_basic.h # gemeinfreie 8x8-Bitmap-Schrift
└── src/
    ├── kernel.c        # Einstiegspunkt: Anfragen an Limine, alles verdrahten
    ├── framebuffer.c/.h# rohe Pixel: Punkte, Rechtecke, Bildschirm löschen
    ├── font.c/.h       # liefert Glyph-Bitmaps für ein Zeichen
    ├── console.c/.h    # Textausgabe: Cursor, Zeilenumbruch, Skalierung, Farben
    ├── io.h            # Port-Ein-/Ausgabe (inb/outb) – Basis für Treiber
    ├── serial.c/.h     # COM1-Treiber: Debug-Ausgabe über die serielle Leitung
    ├── isr.S           # Assembler-Stubs für alle Interrupt-Vektoren
    ├── idt.c/.h        # Interrupt Descriptor Table aufsetzen und laden
    ├── interrupts.c/.h # Verteiler: Ausnahmen melden, IRQ-Handler aufrufen
    ├── pic.c/.h        # 8259-PIC umprogrammieren, Interrupts bestätigen
    ├── keyboard.c/.h   # PS/2-Tastatur: Scancodes -> Zeichen (IRQ1)
    ├── mouse.c/.h      # PS/2-Maus: 3-Byte-Pakete, Cursorposition (IRQ12)
    └── desktop.c/.h    # Fenster, Taskleiste, Mauszeiger, Ziehen per Maus
```

Der **Framebuffer** zeichnet jetzt in einen Back-Buffer; `fb_present()` kopiert
ihn auf einen Schlag auf den Bildschirm (kein Flackern). Die **Console** kann
mit `console_draw_text()` Text an beliebigen Pixelpositionen ausgeben.

Die Module sind sauber getrennt:
**framebuffer** kennt nur Pixel, **font** nur Glyphen, **console** kombiniert
beide zu Text, und **kernel** verbindet alles mit dem Bootloader. Neue Module
(Tastatur, Speicherverwaltung, Interrupts ...) hängst du genauso an.

## Voraussetzungen

- `clang` und `ld.lld` (LLVM ist von Haus aus ein Cross-Compiler – keine
  eigene GCC-Toolchain nötig)
- `xorriso` zum Bauen des ISO-Images
- `qemu-system-x86_64` zum Testen
- der **Limine**-Bootloader (siehe unten)

Auf Debian/Ubuntu:
```
sudo apt install clang lld xorriso qemu-system-x86
```

## Limine besorgen

```
git clone https://github.com/limine-bootloader/limine.git \
    --branch=v8.x-binary --depth=1
make -C limine        # baut das kleine 'limine'-Host-Tool
```

Lege den `limine`-Ordner neben `blakeos/` ab (Standard), oder gib den Pfad beim
Bauen an: `make LIMINE_DIR=/pfad/zu/limine`.

## Bauen und starten

```
make            # kompiliert den Kernel und erzeugt blakeos.iso
make run        # startet das ISO in QEMU (BIOS-Modus)
```

Für den UEFI-Modus brauchst du OVMF-Firmware (`sudo apt install ovmf`):
```
make run-uefi
```

Die serielle Debug-Ausgabe siehst du im Terminal mit:
```
make run-debug
```

Hinweis: Unter **Wayland** reicht QEMU relative Mausbewegung oft nicht durch
(man sieht zwei Cursor, der Gast-Cursor bewegt sich nicht). Deshalb starten die
`make run`-Befehle QEMU mit `GDK_BACKEND=x11`. Falls die Maus dennoch klemmt,
hilft alternativ `-display sdl`.

## Wie es funktioniert (Kurzfassung)

1. Der Kernel legt im Abschnitt `.limine_requests` Strukturen ab, die Limine vor
   dem Start liest – darunter die Bitte um einen Framebuffer.
2. Limine richtet den 64-Bit-Long-Mode samt Paging ein und springt nach `kmain`.
3. `kmain` holt die Framebuffer-Adresse, Breite, Höhe und `pitch` aus der Antwort.
4. Wir schreiben 32-Bit-Pixel direkt in den Speicher – das ist die ganze „Grafik".
5. Die Konsole zeichnet pro Zeichen ein 8x8-Glyph, skaliert auf 24x24 Pixel.

## Nächste sinnvolle Schritte

- [x] **Serielle Ausgabe** (COM1) zum Debuggen – erledigt (`serial.c`).
- [x] **IDT + Interrupts** und **PS/2-Tastaturtreiber** – erledigt
      (`isr.S`, `idt.c`, `interrupts.c`, `pic.c`, `keyboard.c`). Tippen
      erscheint live auf dem Bildschirm und im Serien-Log.
- [x] **Maus (IRQ12) + Desktop** – erledigt (`mouse.c`, `desktop.c`).
      Double-Buffering, Mauszeiger und ein per Maus ziehbares Fenster.
- [ ] **GDT** neu setzen – optional; Limine liefert bereits eine brauchbare.
      Wird erst für eine TSS / den Wechsel in den Userspace wirklich nötig.
- [ ] **Physische Speicherverwaltung** (Limine liefert eine Memory-Map mit) –
      nötig für mehrere Fenster/Programme statt nur einem festen.
- [ ] **Timer (IRQ0)** + **Multitasking**, dann Dateisystem, ELF-Loader ...

Das OSDev-Wiki (https://wiki.osdev.org) und die Limine-Beispiele
(https://github.com/limine-bootloader/limine-c-template) sind die besten
Begleiter für jeden dieser Schritte.
