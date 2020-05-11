#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "secuencial.h"
#include "utils.h"

#define SERVER_NAME "ForkedServer"

int socket_file_descriptor;

void sigint_handler(int sig_num) {
  printf("CTRL+C fue presionado, el servidor debe terminar\n");
  tcp_connection_uninit(socket_file_descriptor);
  exit(EXIT_SUCCESS);
}

int init_server(int port){
    socket_file_descriptor = tcp_connection_init(port, NULL, true);
    if (socket_file_descriptor < 0) {
      fprintf(stderr, "Error inicializando servidor\n");
      return -1;
    }

    return socket_file_descriptor;
}

int execute_sequential_server(int port_int, char *root){
    // struct sockaddr_in is defined in in.h
    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(client_address);
    int accept_response;

    printf("El servidor ha iniciado en el puerto No. %d con el directorio raiz"
           " %s\n", port_int, root);

    printf("Inicializando servidor...\n");
    init_server(port_int);

    if (socket_file_descriptor < 0) {
        fprintf(stderr, "Error inicializando el servidor. (Errno. %s)\n", strerror(errno));
        return EXIT_FAILURE;
    }
    printf("Servidor inicializado\n");

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
      fprintf(stderr, "Error %d al instalar el handler para SIGINT\n", errno);
      return EXIT_FAILURE;
    }

    printf("Presione CTRL+C para terminar el programa\n");

    while (1) {
        printf("Esperando solicitud...\n");
        accept_response = accept(socket_file_descriptor, (struct sockaddr*)&client_address, &client_address_length);
        printf("Solicitud recibida\n");
        if (accept_response < 0) {
            fprintf(stderr, "Error esperando una conexión de socket en la función accept. (Errno. %s)\n",
                    strerror(errno));
            continue;
        }
        respond_to_request(root, accept_response, SERVER_NAME);
    }
}
