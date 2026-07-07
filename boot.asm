; Defino constantes para el estándar Multiboot
MODULEALIGN equ  1<<0
MEMINFO     equ  1<<1
FLAGS       equ  MODULEALIGN | MEMINFO
MAGIC       equ  0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
resb 16384 ; Reservamos 16 KB para la pila (stack)
stack_top:

section .text
global _start
extern kmain ; Declaramos que kmain está en otro archivo (C++)

_start:
    mov esp, stack_top ; Configuramos el puntero de la pila
    call kmain         ; ¡Saltamos a C++!
    cli                ; Si C++ termina, desactivamos interrupciones
.hang: hlt             ; Y congelamos el CPU
    jmp .hang
