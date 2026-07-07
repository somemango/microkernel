; funciones para detectar el teclado, entrada de valor de la tecla pulsada
global inb  ; Permite que C++ vea esta función
global outb

inb:
    mov dx, [esp + 4] ; Toma el puerto de los argumentos de la función
    in al, dx         ; Lee el dato del hardware
    ret

outb:
    mov dx, [esp + 4] ; Puerto
    mov al, [esp + 8] ; Dato a enviar
    out dx, al        ; Envía el dato
    ret

    global teclado_handler_wrapper
    extern teclado_handler_main

teclado_handler_wrapper:
    pushad              ; Guarda todos los registros (EAX, ECX, etc.)
    call teclado_handler_main
    popad               ; Restaura los registros
    iretd               ; Instrucción especial para volver de una interrupción

global cargar_idt
cargar_idt:
    mov eax, [esp + 4]    ; Toma la dirección de 'idtp' que pasamos desde C++
    lidt [eax]            ; Carga la IDT
    ret
