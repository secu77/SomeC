/*
 *   FileTransfer Version 1.0 (Server)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#define PUERTO 9999		//Puerto de escucha por default
#define Ncox 2




/*
 *  Función que obtiene el tamaño en bytes de un archivo pasándole su ruta, 
 *  y haciendo uso de la función del SO stat.
 *
 *  Devuelve el tamaño total en un long.
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
 *  [M] Funcion Principal.
 *
 *  Futuras implementaciones:
 *    - Solicitar en primer mensaje el archivo a transmitir.
 *    - Optimización del Programa.
 *    - Estimación de tiempo restante durante la ejecución.
 *    - Cifrado y de datos a transmitir.
 */
int main(int argc, char *argv[]){

	struct sockaddr_in cliente, servidor;
	int lensckadd = sizeof(struct sockaddr);
	int recsocket = 0, recsock = 0;
	int conectado = 0, pid = 0;
	unsigned int lens = sizeof(struct sockaddr_in);
	char *mensaje;
	int descriptor;
	long numbytes, bytesread;
	char path[]="doc.html";				// Archivo que se transfiere, modificar si es necesario. (ejemplo)

	/*
	 * Validando argumentos introducidos.
	 */
	if(argc != 1){
	  printf("\n[+]Uso: %s\n\n", argv[0]);
	  exit(0);
	}

	// Creando un socket del tipo INET, para conexiones no internas.
	if((recsocket = socket(AF_INET, SOCK_STREAM,0)) == -1){
	  printf("\n[!]Error abriendo el socket\n");
	  exit(-1);
	}

	// Añadiendo el tipo de socket, el puerto y la direccion ip.
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(PUERTO);
	servidor.sin_addr.s_addr = INADDR_ANY;
	memset(servidor.sin_zero,0,8);


	// Asociando el socket con el puerto correspondiente de la dirección de la máquina.
	if(bind(recsocket,(struct sockaddr*) &servidor, lensckadd) == -1){
	  printf("\n[!]Error en la fase de asociacion\n");
	  exit(-1);
	}

	// Poniendo a la escucha y estableciendo el numero de conexiones que aceptará.
	if(listen(recsocket,Ncox) == -1){
	  printf("\n[!]Error en la fase de escucha\n");
	  exit(-1);
	}


	// Bucle principal, se mantiene a la espera hasta que se conecta el primer cliente.
	while(!conectado){
	  
	  printf("\n[/]Esperando conexion...\n");

	  // Aceptando la conexiones entrante cuando un cliente se conecta.
	  if((recsock = accept(recsocket,(struct sockaddr *) &cliente, &lens)) == -1){
	    printf("\n[!]Error en la fase de aceptacion\n");
		exit(-1);
	  }else{
	    
	    printf("  [*]Se conectó un cliente con IP:%s\n",inet_ntoa(cliente.sin_addr));

		/*
		 * Creando el hijo que será el encargado de recibir leer los bloques del archivo a transferir,
		 * y de procesar el envio al cliente.
		 */
		pid = fork();

		if(pid == -1){
		  printf("\nError hijo\n");
		  exit(-1);
		}else if(pid == 0){

		 /*
		  *  Obtengo el número de bytes del fichero, por si hay algún tipo de error.
		  *
		  *  También puede usarse para hacer un cálculo o estimación del tiempo que se tardará en
		  *  enviar el archivo completo y plasmarlo en una barra de tiempo restante.
		  */
		  numbytes = getBytes(path);
		  if(numbytes < 0){
		    printf("\nError al determinar el numero de bytes del archivo %s\n",path);
		    exit(-1);
		  }

		  // Reservo un bloque de 1024 char's que son un total de unos 1024 bytes para el buffer.
		  mensaje = (char *)malloc(1024/sizeof(char));
          
          // Apertura del archivo en modo lectura.
          descriptor = open(path, O_RDONLY);
          if(descriptor < 0){
            printf("\nError al abrir el archivo %s\n",path);
            exit(-1);
          }
          
          // Pequeño Header con información básica. (Bytes del archivo)
          printf("\n[+]Se enviará el archivo %s con %ld bytes de tamaño en %ld bloques de 1024 bytes\n",path,numbytes,(numbytes/1024));
          printf("\n [~]Enviando [");

          /*
           *  Se hace una lectura continua de bloques de 1024 bytes.
           *  Una buena practica es utilizar este tamaño o bloques de
           *  512 bytes, para no tener problemas a la hora de transmitir
           *  los bloques hacia el cliente.
           */
          while((bytesread = read(descriptor,mensaje,(long)1024)) != 0){

            // Le mando el bloque de bytes que leo, que, por defecto son 1024 bytes, aunque pueden ser menos (en el último bloque). 
		    send(recsock,mensaje,bytesread,0);

		    printf("-");
          }

          printf(">]\t(*)Enviado el archivo completo\n");

		  free(mensaje);
		  exit(0);
		}else{
	      wait(NULL);
		  conectado = 1;
		}
	  }
		close(recsocket);
		close(recsock);
	}
	return 0;
}