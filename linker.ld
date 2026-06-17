/* Ausgabeformat und Einstiegspunkt */
OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

/* Programm-Header: getrennte Segmente mit passenden Rechten */
PHDRS
{
    limine_requests PT_LOAD;
    text            PT_LOAD;
    rodata          PT_LOAD;
    data            PT_LOAD;
}

SECTIONS
{
    /* Higher-half: oberste 2 GiB des virtuellen Adressraums. */
    . = 0xffffffff80000000;

    /* Die Limine-Anfragen müssen früh und am Stück liegen. */
    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .text : {
        *(.text .text.*)
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data : {
        *(.data .data.*)
    } :data

    /* Uninitialisierte Daten */
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /* Nicht benötigte Abschnitte verwerfen */
    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}
