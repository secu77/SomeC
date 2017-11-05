/*
 *  Copiar.c Version 1.1
 *
 *  Este programa te permite hacer un duplicado de un archivo pasado por argumentos a otro destino, 
 *  también pasado por argumento. 
 *  El programa determina los bytes del archivo a copiar, hace una lectura continua en bloques de 1024 bytes,
 *  y los va copiando en la tuberia, de igual forma el padre va leyendo en bloques de 1024 bytes
 *  y escribiendo dicho bloque de bytes en el archivo final.
 *
 *  El procedimiento es: El padre crea un hijo que lee el archivo bytes a bytes (en bloques de 'LENBLOCK'),
 *  los pasa a la tuberia con la que se comunica con el padre, y este finalmente escribe dichos bloques en
 *  el archivo resultante. Va mostrando (de forma muy simple) como va cada bloque.
 *
 *  Se ha solucionado el problema de reservar un bloque de memoria demasiado grande.
 *
 *  A espera de revisiones y mejoras.
 *
 *  Secu, de StationX11
 *
 *  Agradecimientos a Dxr (de CrackSLatinoS) y KiloKang (de Fwhibbit) por su ayuda y consejo!
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#define LENBLOCK 1024   // Tamaño del Bloque de Bytes.

/*
 *  Función que obtiene el tamaño en bytes de un archivo pasandole la ruta de este.
 *  devuelve el tamaño total en un long.
 */	
long getBytes(char *path){
  struct stat buff;

  if(stat(path,&buff) < 0){
  	printf("\nError al usar stat sobre el fichero\n");
  	return -1;
  }else{
  	return buff.st_size;
  }
}


/*
 *  Función principal, o funcion Main.
 */
int main(int argc, char **argv){

  char *buffer;
  char *bufferout;
  char *path;
  char *outpath;
  long numbytes = 0, readpipe = 0;
  int descriptor, outcriptor, reader, pid;
  int fd[2];


  /*
   *  Validando argumentos introducidos en el inicio.
   */	
  if(argc == 3){
    path = argv[1];
    outpath = argv[2];

    pipe(fd);

    /*
     *  Validando la obtención del número de bytes de la ruta del archivo.
     */
     numbytes = getBytes(path);
     if(numbytes < 1){
       printf("\nError al determinar el numero de bytes del archivo %s\n",path);
       exit(-1);
     } 


    /*
     *  Creacion de Procesos. (Explicado en el header)
     */
    pid = fork();
    switch(pid){

      case -1:
        printf("\nError al crear el proceso hijo\n");
        exit(-1);
        break;

      /*
       *  Proceso Hijo
       */
      case 0:
      
        close(fd[0]);                                      // Cerrando el modo lectura de la tuberia.
        buffer = (char *)malloc(LENBLOCK/sizeof(char));    // Reservo 1024 bytes es decir, espacio para 1024 chars.

        // Validando la apertura del archivo pasado por argumentos
        descriptor = open(path, O_RDONLY);
        if(descriptor < 0){
          printf("\nError al abrir el archivo %s\n",path);
          exit(-1);
        }

        printf("\nCopiando: [");
        // Leyendo del fichero 1 y escribiendo los bloques en la tuberia.
        while((reader = read(descriptor,buffer,LENBLOCK)) != 0){
          //printf("\nBuffer:{%s}\n", buffer);
          write(fd[1],buffer,reader);
          printf("-");
        }

        printf(">]\t (*)Archivo copiado con éxito!\n\n");

        close(descriptor);
        exit(0);

        break;

      /*
       *  Proceso Padre
       */
      default:

      close(fd[1]);                                        // Cerrando el modo escritura en la tuberia.
      wait(NULL);                                          // Esperando a que el hijo finalice su escritura en la tuberia.
      bufferout = (char*)malloc(LENBLOCK/sizeof(char));    // Reservando un bloque de 1024 bytes como antes para el buffer.

      // Lectura por bloques hasta el último.
      while((readpipe = read(fd[0],bufferout,LENBLOCK)) != 0){

        // Validando la apertura/creacion del archivo de destino.
        outcriptor = open(outpath, O_WRONLY|O_CREAT|O_APPEND, 0666);
        if(outcriptor < 0){
          printf("\nError al crear el archivo de destino %s\n",outpath);
          exit(-1);
        }

        // Escribiendo en el archivo el bloque y poniendo a cero la memoria del buffer.
        write(outcriptor,bufferout,readpipe);
        memset(bufferout,0,LENBLOCK);
      }

      close(outcriptor);  
    }

  }else{
    printf("\nError con los argumentos\nUso: ej2_so archivoacopiar nuevoarchivo\n");
  }

  return 0;
}
