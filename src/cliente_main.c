#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "cliente.h"

#define BUFFSIZE	256

int client(char *address, int port)
{
	int client_socket;
	

	// connect to an address
	struct sockaddr_in remote_address;
	
	memset(&remote_address,0,sizeof(remote_address));
	remote_address.sin_family = AF_INET;
	remote_address.sin_port = htons(port);
	remote_address.sin_addr.s_addr = inet_addr(address);
	//inet_aton(address, &remote_address.sin_addr.s_addr);

	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0))<0){
		perror("socket client_socket");
		return 1;
	}
	
	
	if (connect(client_socket, (struct sockaddr *) &remote_address, sizeof(remote_address))< 0)
	{
		perror("connect client_socket");
		return 1;
	}

	printf("connected to server\n");

	char request[] = "GET / HTTP/1.1\r\n\r\n";
	char response[4096];

	send(client_socket, request, sizeof(request), 0);
	recv(client_socket, &response, sizeof(response), 0);

	printf("response from the server: %s\n", response);
	close(client_socket);

	return 0;
}

int main(int argc, char *argv[]) {
  int   opt;
  char *maquina = NULL;
  char *archivo = NULL;
  int   puerto  = -1;
  int   threads = -1;
  int   ciclos  = -1;

  while ((opt = getopt(argc, argv, "m:p:a:t:c:")) != -1) {
      switch (opt) {
          case 'm':
              maquina = optarg;
              break;
          case 'p':
              puerto = atoi(optarg);
              break;
          case 'a':
              archivo = optarg;
              break;
          case 't':
              threads = atoi(optarg);
              break;
          case 'c':
              ciclos = atoi(optarg);
              break;
          default:
              fprintf(stderr, "Uso: %s -m nombre_ip_maquina_servidor -p puerto"
                      " -a archivo_solicitado -t número_threads"
                      " -c número_ciclos\n", argv[0]);
              return EXIT_FAILURE;
      }
  }

  if (maquina == NULL) {
      fprintf(stderr, "-m nombre_ip_maquina_servidor es un parámetro obligatorio\n");
      return EXIT_FAILURE;
  }

  if (archivo == NULL) {
      fprintf(stderr, "-a archivo_solicitado es un parámetro obligatorio\n");
      return EXIT_FAILURE;
  }

  if (puerto == -1) {
      fprintf(stderr, "-p puerto es un parámetro obligatorio\n");
      return EXIT_FAILURE;
  } else if (puerto < 0) {
      fprintf(stderr, "-p puerto debe ser positivo\n");
      return EXIT_FAILURE;
  }

  if (threads == -1) {
      fprintf(stderr, "-t número_threads es un parámetro obligatorio\n");
      return EXIT_FAILURE;
  } else if (threads < 0) {
      fprintf(stderr, "-t número_threads debe ser positivo\n");
      return EXIT_FAILURE;
  }

  if (ciclos == -1) {
      fprintf(stderr, "-c número_ciclos es un parámetro obligatorio\n");
      return EXIT_FAILURE;
  } else if (ciclos < 0) {
      fprintf(stderr, "-c número_ciclos debe ser positivo\n");
      return EXIT_FAILURE;
  }

  printf("\n\n  ______   __  __                       __\n");
  printf(" /      \\ |  \\|  \\                     |  \\\n");
  printf("|  $$$$$$\\| $$ \\$$  ______   _______  _| $$_     ______\n");
  printf("| $$   \\$$| $$|  \\ /      \\ |       \\|   $$ \\   /      \\\n");
  printf("| $$      | $$| $$|  $$$$$$\\| $$$$$$$\\\\$$$$$$  |  $$$$$$\\\n");
  printf("| $$   __ | $$| $$| $$    $$| $$  | $$ | $$ __ | $$    $$\n");
  printf("| $$__/  \\| $$| $$| $$$$$$$$| $$  | $$ | $$|  \\| $$$$$$$$\n");
  printf(" \\$$    $$| $$| $$ \\$$     \\| $$  | $$  \\$$  $$ \\$$     \\\n");
  printf("  \\$$$$$$  \\$$ \\$$  \\$$$$$$$ \\$$   \\$$   \\$$$$   \\$$$$$$$\n\n");

  printf("%s ejecutando %d threads que solicitan el archivo %s %d veces "
         "al servidor %s en el puerto %d\n", argv[0], threads, archivo, ciclos,
         maquina, puerto);
         
  client(maquina,puerto);

  return EXIT_SUCCESS;
}
