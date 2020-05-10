#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include "forked.h"
#include "utils.h"

#define SERVER_NAME "ForkedServer"

int init_server(char *port){
    int socket_file_descriptor;
    //Type addrinfo is defined in netdb.h
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *p;
    int getaddrinfo_response;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo_response = getaddrinfo(NULL, port, &hints, &res);
    if (getaddrinfo_response != 0) {
        fprintf(stderr, "Error en la función getaddrinfo_response. (Errno. %s)\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (p = res; p != NULL; p = p->ai_next) {
        socket_file_descriptor = socket(p->ai_family, p->ai_socktype, 0);
        if (socket_file_descriptor == -1) {
            continue;
        }
        if (bind(socket_file_descriptor, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }
    }
    if (p == NULL) {
        fprintf(stderr, "Error en la función socket o bind. (Errno. %s)\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    if (listen(socket_file_descriptor, SOMAXCONN) != 0) {
        fprintf(stderr, "Error en la función listen. (Errno. %s)\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    return socket_file_descriptor;
}

int execute_forked_server(int port_int, char *root){
    // struct sockaddr_in is defined in in.h
    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(client_address);
    char port_char[6];
    int socket_file_descriptor;
    int accept_response;
    int fork_response;

    sprintf(port_char, "%d", port_int);
    printf("El servidor ha iniciado en el puerto No. %s con el directorio raiz %s\n", port_char, root);

    printf("Inicializando servidor...\n");
    socket_file_descriptor = init_server(port_char);

    if (socket_file_descriptor < 0) {
        fprintf(stderr, "Error inicializando el servidor. (Errno. %s)\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Servidor inicializado\n");

    while (1) {
        printf("Esperando solicitud...\n");
        accept_response = accept(socket_file_descriptor, (struct sockaddr*)&client_address, &client_address_length);
        printf("Solicitud recibida\n");
        if (accept_response < 0) {
            fprintf(stderr, "Error esperando una conexión de socket en la función accept. (Errno. %s)\n",
                    strerror(errno));
            continue;
        }

        fork_response = fork();
        if (fork_response == 0) {
            printf("Un nuevo proceso se ha creado para atender a %s. PID: %d\n",
                   inet_ntoa(client_address.sin_addr), getpid());
            respond_to_request(root, accept_response, SERVER_NAME);
            printf("Terminando proceso que atendió a %s\n", inet_ntoa(client_address.sin_addr));
            exit(EXIT_SUCCESS);
        }

        if (fork_response < 0) {
            fprintf(stderr, "Error ejecutando la función fork. (Errno. %s)\n",
                    strerror(errno));
            continue;
        }
        printf("Se creó el proceso con PID: %d\n", fork_response);
    }
}