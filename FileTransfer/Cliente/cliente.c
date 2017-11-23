/*
 *   FileTransfer Version 1.0 (Cliente)
 *
 *  Este programa te permite transferir un archivo de una maquina donde se situa el servidor a otra
 *  donde esta el cliente que lo solicita.
 *
 *  El programa determina los bytes del archivo a copiar, hace una lectura continua en bloques de 1024 bytes,
 *  y los va copiando en la tuberia, de igual forma el padre va leyendo en bloques de 1024 bytes
 *  y escribiendo dicho bloque de bytes en el archivo final sin previa espera a que finalice el hijo la escritura.
 *
 *  El procedimiento es: El padre crea un hijo que lee el archivo en bloques de 1024 bytes, y
 *  los va enviando a traves del socket al cliente, mostrando por pantalla el proceso. El cliente, se encarga
 *  de recibir dichos bloques de bytes que contienen el archivo fragmentado que manda el servidor, y va
 *  escribiendo cada bloque en un archivo final.
 *
 *  A espera de revisiones y mejoras.
 *
 *  Secu, de StationX11
 *
 *  Agradecimientos a Dxr (de CrackSLatinoS) por su ayuda!
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define PUERTO 9999
#define MAXSOCKETBYTES 1024



int main(int argc, char *argv[]){

  int fd;
  int outcriptor, writer;
  struct hostent *rec;
  struct sockaddr_in servidor;
  int recHi = 0;
  char *blockB;
  unsigned int sizebytes;

  blockB = (char *)calloc(MAXSOCKETBYTES,sizeof(char));


  // Validacion de IP por argumentos
  if (argc != 2)
  {
  	printf("\n[*]Uso: %s <SERVER IP>\n", argv[0]);
  	exit(-1);
  }

  if ((rec = gethostbyname(argv[1])) == NULL)
  {
  	printf("\nError con el gethostbyname\n");
  	exit(-1);
  }

  // Creando el socket.
  if ((fd = socket(AF_INET, SOCK_STREAM,0)) == -1)
  {
  	printf("\nError con la creacion del socket\n");
  	exit(-1);
  }


  // Rellenando datos del Servidor.
  servidor.sin_family = AF_INET;							            //Socket no interno, sino a internet
  servidor.sin_port = htons(PUERTO);						          //pasar el puerto a el formato de Bytes de Red
  servidor.sin_addr = *((struct in_addr *)rec->h_addr);		//coloca la ip indicada
  memset(servidor.sin_zero,0,8);							            //Todo a ceros


  // Conectandose al servidor.
  if(connect(fd,(struct sockaddr *) &servidor, sizeof(struct sockaddr)) == -1){
  	printf("\nError intentar conectar\n");
  	exit(-1);
  }


  /*
   *  Se mantiene a la espera hasta que le lleguen todos los bloques de bytes,
   *  del archivo fragmentado, conforme los recibe los escribe.
   *  Cuando deja de recibir, activa el flag y se sale.
   */
  while(!recHi){

    /*
     *  Recibo el bloque de 1024 bytes del socket.
     */
    sizebytes = recv(fd, blockB, 1024,0);

    if(sizebytes < 0){
      printf("\nError al recibir la respuesta del servidor\n");
      exit(-1);
    }else if(sizebytes > 0){
      
      *(blockB+sizebytes) = '\0';

      // Validando la apertura/creacion del archivo de destino.
      outcriptor = open("doc.htm", O_WRONLY|O_CREAT|O_APPEND, 0666);  // Ejemplo de nombre de archivo.
      if(outcriptor < 0){
        printf("\nError al crear el archivo de destino\n");
        exit(-1);
      }

      // Validando la escritura de bytes sobre el archivo destino
      writer = write(outcriptor,blockB,sizebytes);
      if(writer < 0){
        printf("\nError al escribir en el archivo\n");
        exit(-1);
      }

      memset(blockB,0,1024);
    }else{

      printf("\n[*] Se ha escrito correctamente el archivo\n");

      recHi = 1;
    }
    
  }

}