/*
 *  Copia.c Version 1.0
 *
 *  Este programa te permite hacer un duplicado de un archivo pasado por argumentos a otro destino, 
 *  también pasado por argumento. 
 *  El programa determina los bytes del archivo a copiar, reserva un buffer de esa cantidad, y posteriormente
 *  lee dicho archivo, lo traslada al buffer, y del buffer lo escribe al archivo destino. Creando un proceso hijo,
 *  que será el que lea de la tuberia y escriba en el archivo final.
 *
 *  Este programa no satisface el objetivo real, ya que la transmisión debería hacerse por bloques de 'x' bytes,
 *  ya que así te evitas reservar un buffer muy grande de memoria.
 *
 *  Sería funcional para archivos pequeños que copiar, pero no para archivos de gran tamaño.
 *
 *  A espera de revisiones y mejoras.
 *
 *  Secu, de StationX11
 */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

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
  long numbytes = 0;
  buffer = (char *)calloc(numbytes,sizeof(char));
  bufferout = (char *)calloc(numbytes,sizeof(char));
  int descriptor, outcriptor, reader, writer, pid, estado;
  int fd[2];


  /*
   *  Validando argumentos introducidos en el inicio
   */	
  if(argc == 3){
    path = argv[1];
    outpath = argv[2];

    pipe(fd);

    /*
     *  Validando la obtención del número de bytes de la ruta del archivo
     */
     numbytes = getBytes(path);
     if(numbytes < 1){
       printf("\nError al determinar el numero de bytes del archivo %s\n",path);
       exit(-1);
     } 


    /*
     *  Creacion de Procesos:
     *
     *   -El proceso Padre lee el fichero que se le pasa por argumentos, determina el numero de
     *    de bytes a extraer y los escribe en la tuberia.
     *   -El proceso Hijo comprueba si existe el fichero pasado por segundo argumento y si puede
     *    crearlo, lee de la tuberia los bytes pasados por el padre y los escribe en el archivo.
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
        
        /*
	 *  Cerrando el descriptor de la tuberia en modo escritura y leyendo los
	 *  bytes del modo lectura
	 */      
        close(fd[1]);
        read(fd[0],bufferout,numbytes);

        //printf("\nSoy el hijo y he leido de la tuberia %ld bytes\n",numbytes);

        /*
	 *  Validando la apertura/creacion del archivo de destino
	 */
	outcriptor = open(outpath, O_WRONLY|O_CREAT|O_TRUNC, 0622);
	if(outcriptor < 0){
	  printf("\nError al crear el archivo de destino %s\n",outpath);
	  exit(-1);
	}

        /*
         *  Validando la escritura de bytes sobre el archivo destino
         */
        writer = write(outcriptor,bufferout,numbytes);
        if(writer < 0){
      	  printf("\nError al escribir en el archivo %s\n",outpath);
  	  exit(-1);
        }else{
      	  //printf("\n[OK]Todo se escribió genial en el archivo %s\n",outpath);
        }

        /*
         *  Liberando memoria, cerrando descriptores y finalizando el hijo.
         */
        free(bufferout);
        close(outcriptor);
        exit(0);
        break;

      /*
       *  Proceso Padre
       */
      default:
        close(fd[0]);

            /*
	     *  Validando la apertura del archivo pasado por argumentos
	     */
	    descriptor = open(path, O_RDONLY);
	    if(descriptor < 0){
	      printf("\nError al abrir el archivo %s\n",path);
	      exit(-1);
	    }

	    /*
	     *  Validando la lectura de bytes del archivo origen
	     */
	    reader = read(descriptor,buffer,numbytes);
	    if(reader < 0){
	      printf("\nError al leer el archivo %s\n",path);
	      exit(-1);
	    }else{
	      //printf("\n[*]Numero de bytes leidos: %ld\n",numbytes);
	    }
        
        /*
	 *  Escribiendo los bytes del archivo a la tuberia y esperando a que
	 *  el hijo acabe con una salida satisfactoria
	 */        
        write(fd[1], buffer, numbytes);
        //printf("\nEscritos %ld en la tuberia por el padre\n",numbytes);
        wait(&estado);
        //printf("\nMurio el hijo con un exit %d\n", ((estado>>8) & 0xFF));

        /*
         *  Liberando memoria y cerrando descriptores
         */
        close(descriptor);
  	free(buffer);
		    
    }

  }else{
    printf("\nError con los argumentos\nUso: ej2_so archivoacopiar nuevoarchivo\n");
  }

  return 0;
}
