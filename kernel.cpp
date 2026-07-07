#include "funciones.h"

extern "C" void kmain(){
    //se usa 0xB8000 porque es la entrada de texto y colores del sistema
    char *video_memory = (char*)0xB8000;

    //limpio pantalla antes q nada
    int cursor_pos;
    for (int i = 0; i < 2000; i++) {
        video_memory[i * 2] = ' ';
        video_memory[i * 2 + 1] = 0x0A; //mantiene el atributo de color verde
    }
    cursor_pos = 0; //regresa el cursor al inicio de la pantalla


    //mensajes del sistema
    const char *saludo = "Bienvenido al microkernel";

    //es un for que imprime el saludo, las letras y el color
    print(video_memory, saludo);

    //limpia las celdas de ram para que el kernel las use como memoria
    for (int i = 0; i < MAX_ARCHIVOS; i++) {
        disco_virtual[i].activo = false;
        disco_virtual[i].tamano = 0;
        disco_virtual[i].nombre[0] = '\0';
    }

    //el archivo [0] sera el usuario.
    disco_virtual[0].activo = true;
    disco_virtual[0].tamano = 4;

    disco_virtual[0].nombre[0] = 'M';
    disco_virtual[0].nombre[1] = 'a';
    disco_virtual[0].nombre[2] = 'u';
    disco_virtual[0].nombre[3] = '\0';

    //verificacion de si los datos se andan guardando en la ram
    if (disco_virtual[0].activo == true) {
        print(video_memory, "Disco RAM: Operando correctamente");
        print(video_memory, "Usuario:");
        print(video_memory, disco_virtual[0].nombre);
    } else {
        print(video_memory, "Error: El disco RAM no retiene datos.");
    }

    //pequeno salto de linea para estetica
    print(video_memory, "");

    iniciar_entrada_teclado();
    while(1) { __asm__("hlt"); }
}
