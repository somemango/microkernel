#ifndef FUNCIONES_H_INCLUDED
#define FUNCIONES_H_INCLUDED

extern "C" {
    unsigned char inb(unsigned short port);
    void outb(unsigned short port, unsigned char data);
    void print(char *memoria_video, const char *palabra);
    void put_char(char c);
    void procesar_tecla(unsigned char scancode);
    void iniciar_entrada_teclado();
    void reprogramar_pic();

    //definicion de variables para la estructura de los archivos en memoria ram (explicacion en funciones.cpp)
    //FUNCIONES PARA LA MEMORIA (ram por ahora)
    #define MAX_ARCHIVOS 5
    struct archivoVirtual {
        char nombre[32];       // el nombre del archivo (maximo 31 letras + '\0')
        char contenido[512];   // el contenido del archivo (tamano fijo, ej: 512 bytes)
        int tamano;            // cuantos bytes reales estan escritos en el contenido
        bool activo;           // nos dice si el archivo existe o si está "vacio/borrado"
    //549b por archivo, con alineacion 552
    };

    extern archivoVirtual disco_virtual[MAX_ARCHIVOS];
}



#endif // FUNCIONES_H_INCLUDED
