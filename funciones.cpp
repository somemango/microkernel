#include "funciones.h"

//variables universales varias y sus explicaciones
char *video_memory = (char*)0xB8000; //direccion hexadecimal de la memoria de video para imprimir
char command_buffer[128]; //comandos de consola
int buffer_idx =0;
int cursor_pos = 0; //la posicion del cursor inicial (modificada por el usuario)
char scancode_a_ascii[] = { //variables para pasar de una tecla pulsada a un caracter en ascii
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};
//eata estructura es una tabla que maneja las interrupciones y le dice a que parte de la memoria ir si sucede x cosa
struct IDT_entry {
    unsigned short offset_lowerbits; // Bits 0-15 de la dirección de la función
    unsigned short selector;         // Selector del segmento de código (GDT)
    unsigned char zero;              // Siempre es 0
    unsigned char type_attr;         // Atributos (P, DPL, Tipo)
    unsigned short offset_higherbits;// Bits 16-31 de la dirección de la función
} __attribute__((packed)); //nota: ese comando raro hace que c++ ponga la struct de una manera especifica que el procesador entiende
//esto es el puntero hacia la tabla
struct IDT_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

//funcion para imprimir mensajes en pantalla
void print(char *memoria_video, const char *palabra){
    for(int i = 0; palabra[i] != '\0'; i++){
        memoria_video[cursor_pos*2] = palabra[i];
        memoria_video[cursor_pos*2 + 1] = 0x0A;
        cursor_pos++;
    }
    //salto de linea
    cursor_pos = (cursor_pos / 80 + 1) * 80;
}

IDT_entry idt[256]; // La tabla con las 256 interrupciones posibles
IDT_ptr idtp;       // El puntero que cargaremos al CPU

// Esta función externa la crearemos en el .asm
extern "C" void cargar_idt(unsigned int);
extern "C" void teclado_handler_wrapper(); // Viene de funciones.asm

void set_idt_gate(int n, unsigned int handler) {
    idt[n].offset_lowerbits = handler & 0xFFFF;    // Bits bajos
    idt[n].selector = 0x08;                         // Selector de código de tu GDT
    idt[n].zero = 0;
    idt[n].type_attr = 0x8E;                        // 10001110 (Presente, Ring 0, 32-bit Interrupt Gate)
    idt[n].offset_higherbits = (handler >> 16) & 0xFFFF; // Bits altos
}

//FUNCIONES PARA EL TECLADO

//funcion para poner un caracter en la posicion del cursor
void put_char(char c) {
    if (c == '\n') {
        cursor_pos = (cursor_pos / 80 + 1) * 80;
    } else {
        video_memory[cursor_pos * 2] = c;
        video_memory[cursor_pos * 2 + 1] = 0x0A;
        cursor_pos++;
    }
}

//funcion para que el kernel acepte comandos.
void comandos(char *ric){ //la variable del caracter se llama ric por mi bro.
    if(ric[0] == 'c' && ric[1] == '\0'){
        for (int i = 0; i < 2000; i++) {
            video_memory[i * 2] = ' ';
            video_memory[i * 2 + 1] = 0x0A;
        }
        cursor_pos = 0;
    }
    else {
        print(video_memory, " Comando no reconocido");
    }
}

//funcion para procasar cual tecla se pulso
void procesar_tecla(unsigned char scancode) {

    if (scancode > 58) return;
    char letra = scancode_a_ascii[scancode];

    if(letra == '\n'){
        command_buffer[buffer_idx] = '\0';
        comandos(command_buffer);
        buffer_idx = 0;
        cursor_pos = (cursor_pos / 80 + 1) * 80;
    }
    else if (letra != 0) {
        // Si hay espacio en el búfer, guardamos y mostramos
        if (buffer_idx < 127) {
            command_buffer[buffer_idx] = letra;
            buffer_idx++;
            put_char(letra); // Lo seguimos mostrando en pantalla
        }
    }
}

extern "C" void teclado_handler_main() {

    video_memory[158] = '!';
    video_memory[159] = 0x0C;

    // 1. Leemos el scancode directamente del puerto de datos
    unsigned char scancode = inb(0x60);

    // 2. Usamos la lógica de tu imagen para filtrar pulsaciones
    if (!(scancode & 0x80)) {
        // Aquí llamas a tu procesar_tecla para imprimir
        procesar_tecla(scancode);
    }

    // 3. ¡MUY IMPORTANTE!
    // Debemos enviar una señal de "Acknowledge" (EOI) al controlador
    // de interrupciones (PIC) para que sepa que puede enviar la siguiente tecla.
    outb(0x20, 0x20);
}

//funcion para que el kernel sepa que el teclado existe basicamente
extern "C" void iniciar_entrada_teclado() {
    while (inb(0x64) & 1) { inb(0x60); }
    reprogramar_pic(); // 1. Movemos las interrupciones a un lugar seguro

    outb(0x64, 0xAE); // Comando para habilitar la interfaz del teclado

    // Limpia el buffer del teclado al iniciar
    while (inb(0x64) & 1) {
        inb(0x60);
    }

    // 1. Configuramos el puntero de la IDT
    idtp.limit = (sizeof(IDT_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    // 2. Registramos el handler del teclado en la interrupción 33 (IRQ 1)
    set_idt_gate(33, (unsigned int)teclado_handler_wrapper);

    // 3. Cargamos la IDT en el procesador (función que haremos en ASM)
    cargar_idt((unsigned int)&idtp);

    // 4. Habilitamos las interrupciones en el CPU
    __asm__ __volatile__("sti");
}


//donde diga MAX_ARCHIVOS, ahi sera 5 ****OJO NUMERO MAXIMO DE ARCHIVOS
#define MAX_ARCHIVOS 5
//vector para un maximo de 5 archivps en ram (aprox 552b c/u entre todos los datos del archivo)
archivoVirtual disco_virtual[MAX_ARCHIVOS];

void reprogramar_pic() {
    // ICW1: Iniciar inicialización del PIC principal y secundario
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW2: Definir el "Vector Offset" (Donde empiezan las interrupciones)
    // El PIC1 empezará en la interrupción 32 (0x20)
    outb(0x21, 0x20);
    // El PIC2 empezará en la interrupción 40 (0x28)
    outb(0xA1, 0x28);

    // ICW3: Configurar la conexión entre los dos PICs
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4: Establecer modo 8086
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // 0xFD es 11111101 en binario (el bit 1 es el teclado, 0 significa habilitado)
    outb(0x21, 0xFD);
    // 0xFF es 11111111 (bloquea todo en el PIC secundario)
    outb(0xA1, 0xFF);
}
