.intel_syntax noprefix
.code64

# ---------------------------------------------------------------------------
# Makros für die Einsprungpunkte (Stubs).
# Manche CPU-Ausnahmen legen selbst einen Fehlercode auf den Stack, andere
# nicht. Damit der Stack-Aufbau für alle gleich ist, pushen wir bei den
# "NOERR"-Varianten eine 0 als Platzhalter.
# ---------------------------------------------------------------------------
.macro ISR_NOERR num
isr_stub_\num:
    push 0
    push \num
    jmp isr_common
.endm

.macro ISR_ERR num
isr_stub_\num:
    push \num
    jmp isr_common
.endm

# CPU-Ausnahmen 0–31. Fehlercode liegt bei 8,10,11,12,13,14,17,21 vor.
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

# Hardware-IRQs 32–47 (nach dem PIC-Remap). Nie mit Fehlercode.
ISR_NOERR 32
ISR_NOERR 33
ISR_NOERR 34
ISR_NOERR 35
ISR_NOERR 36
ISR_NOERR 37
ISR_NOERR 38
ISR_NOERR 39
ISR_NOERR 40
ISR_NOERR 41
ISR_NOERR 42
ISR_NOERR 43
ISR_NOERR 44
ISR_NOERR 45
ISR_NOERR 46
ISR_NOERR 47

# ---------------------------------------------------------------------------
# Gemeinsame Routine: sichert alle Register, ruft die C-Funktion isr_handler
# mit einem Zeiger auf die gesicherten Register auf, stellt alles wieder her.
# ---------------------------------------------------------------------------
isr_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp            # 1. Argument: Zeiger auf registers_t
    call isr_handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16             # int_no und err_code verwerfen
    iretq

# ---------------------------------------------------------------------------
# Tabelle mit den Adressen aller Stubs – die IDT-Initialisierung in C liest
# sie aus.
# ---------------------------------------------------------------------------
.section .rodata
.align 8
.global isr_stub_table
isr_stub_table:
.irp num,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47
    .quad isr_stub_\num
.endr
