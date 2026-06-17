# ===== Konfiguration =========================================================
# Pfad zum Limine-Verzeichnis (Binär-Distribution + gebautes 'limine'-Tool).
LIMINE_DIR ?= ../limine

CC      := clang
LD      := ld.lld

# Freestanding-Kernel: keine Standardbibliothek, kein FPU/SSE, kein Red Zone.
CFLAGS := \
    --target=x86_64-unknown-none \
    -ffreestanding -fno-stack-protector -fno-stack-check \
    -fno-PIC -fno-PIE \
    -m64 -march=x86-64 -mcmodel=kernel \
    -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone \
    -Iinclude -Isrc \
    -Wall -Wextra -std=c11 -O2 -g

LDFLAGS := \
    -m elf_x86_64 -nostdlib -static \
    -z max-page-size=0x1000 \
    -T linker.ld

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)

KERNEL := blakeos.elf
ISO    := blakeos.iso

# ===== Regeln ================================================================
.PHONY: all clean run run-uefi

all: $(ISO)

# Jede .c-Datei zu einer .o-Datei kompilieren.
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Kernel-ELF aus allen Objektdateien linken.
$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(KERNEL)

# Bootfähiges ISO-Image bauen (BIOS + UEFI).
$(ISO): $(KERNEL) limine.conf
	rm -rf iso_root
	mkdir -p iso_root/boot/limine iso_root/EFI/BOOT
	cp $(KERNEL)        iso_root/boot/kernel
	cp limine.conf      iso_root/boot/limine/
	cp $(LIMINE_DIR)/limine-bios.sys      iso_root/boot/limine/
	cp $(LIMINE_DIR)/limine-bios-cd.bin   iso_root/boot/limine/
	cp $(LIMINE_DIR)/limine-uefi-cd.bin   iso_root/boot/limine/
	cp $(LIMINE_DIR)/BOOTX64.EFI          iso_root/EFI/BOOT/
	cp $(LIMINE_DIR)/BOOTIA32.EFI         iso_root/EFI/BOOT/
	xorriso -as mkisofs -R -r -J \
	    -b boot/limine/limine-bios-cd.bin \
	    -no-emul-boot -boot-load-size 4 -boot-info-table \
	    -hfsplus -apm-block-size 2048 \
	    --efi-boot boot/limine/limine-uefi-cd.bin \
	    -efi-boot-part --efi-boot-image \
	    --protective-msdos-label \
	    iso_root -o $(ISO) 2>/dev/null
	# BIOS-Bootcode in das ISO schreiben.
	$(LIMINE_DIR)/limine bios-install $(ISO)

# Im Emulator starten (BIOS-Modus).
run: $(ISO)
	qemu-system-x86_64 -M q35 -m 256M -cdrom $(ISO) -boot d

# Im Emulator starten (UEFI-Modus, benötigt OVMF-Firmware).
run-uefi: $(ISO)
	qemu-system-x86_64 -M q35 -m 256M \
	    -bios /usr/share/ovmf/OVMF.fd -cdrom $(ISO) -boot d

clean:
	rm -rf $(OBJS) $(KERNEL) $(ISO) iso_root
