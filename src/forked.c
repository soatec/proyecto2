#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/limits.h>
#include "forked.h"
#include "utils.h"

#define WRITE_BUFFER_SIZE 1024
#define READ_BUFFER_SIZE 212992

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
        fprintf(stderr, "Error en la función getaddrinfo_response. (Err no. %s)\n",
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
        fprintf(stderr, "Error en la función socket o bind. (Err no. %s)\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);
    if (listen(socket_file_descriptor, SOMAXCONN) != 0) {
        fprintf(stderr, "Error en la función listen. (Err no. %s)\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    return socket_file_descriptor;
}

void close_connection(int file_descriptor) {
    // SHUT_WR = No more transmissions
    shutdown(file_descriptor, SHUT_RDWR);
    close(file_descriptor);
}

void respond_to_get(char *root, int file_descriptor) {
    char mesg[READ_BUFFER_SIZE];
    // A request line has three parts, separated by spaces: a method name,
    // the local path of the requested resource, and the version of HTTP being used
    char *required_strings[3];
    char data_to_send[WRITE_BUFFER_SIZE];
    char path[PATH_MAX];
    int recv_responde;
    int get_file_descriptor;
    int bytes_read;

    memset((void*)mesg, (int)'\0', READ_BUFFER_SIZE);
    recv_responde = recv(file_descriptor, mesg, READ_BUFFER_SIZE, 0);

    if (recv_responde < 0) {
        fprintf(stderr, ("Error al recibir el mensaje\n"));
        close_connection(file_descriptor);
        return;
    }
    if (recv_responde == 0) {
        fprintf(stderr, "El cliente se desconectó\n");
        close_connection(file_descriptor);
        return;
    }

    printf("Mensaje:\n\n%sFin del mensaje\n", mesg);
    required_strings[0] = strtok(mesg, " \t\n");
    if (strncmp(required_strings[0], "GET\0", 4) == 0) {
        required_strings[1] = strtok (NULL, " \t");
        required_strings[2] = strtok (NULL, " \t\n");
        if (strncmp(required_strings[2], "HTTP/1.1", 8) != 0) {
            write(file_descriptor, "HTTP/1.1 400 Bad Request\n", 25);
            close_connection(file_descriptor);
            return;
        }
        if (strncmp(required_strings[1], "/\0", 2) == 0) {
            required_strings[1] = "/index.html";
        }
        strcpy(path, root);
        strcpy(&path[strlen(root)], required_strings[1]);
        printf("Archivo: %s\n", path);
        get_file_descriptor = open(path, O_RDONLY);
        if (get_file_descriptor != -1) {
            printf("Archivo encontrado\n");
            send(file_descriptor, "HTTP/1.1 200 OK\n\n", 17, 0);
            while ((bytes_read=read(get_file_descriptor, data_to_send, WRITE_BUFFER_SIZE)) > 0){
                write(file_descriptor, data_to_send, bytes_read);
            }
        } else {
            write(file_descriptor, "HTTP/1.1 404 Not Found\n", 23);
            printf("Archivo no encontrado\n");
        }
    }
    close_connection(file_descriptor);
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
        fprintf(stderr, "Error inicializando el servidor. (Err no. %s)\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Servidor inicializado\n");

    while (1) {
        printf("Esperando solicitud...\n");
        accept_response = accept(socket_file_descriptor, (struct sockaddr*)&client_address, &client_address_length);
        printf("Solicitud recibida\n");
        if (accept_response < 0) {
            fprintf(stderr, "Error esperando una conexión de socket en la función accept. (Err no. %s)\n",
                    strerror(errno));
            continue;
        }

        fork_response = fork();
        if (fork_response == 0) {
            printf("Un nuevo proceso se ha creado para atender a %s. PID: %d\n",
                   inet_ntoa(client_address.sin_addr), getpid());
            respond_to_get(root, accept_response);
            printf("Terminando proceso que atendió a %s\n", inet_ntoa(client_address.sin_addr));
            exit(0);
        }

        if (fork_response < 0) {
            fprintf(stderr, "Error ejecutando la función fork. (Err no. %s)\n",
                    strerror(errno));
            continue;
        }

        printf("Se creó el proceso con PID: %d\n", fork_response);
    }
}