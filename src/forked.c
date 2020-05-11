#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "forked.h"
#include "utils.h"

#define SERVER_NAME "ForkedServer"

int init_forked_server(int port_int){
  int fd;
  int shmem_fd;
  bool *run;

  // Abrir memoria compartida en modo read/write
  shmem_fd = shm_open("forked_shmem",
                      O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

  if (shmem_fd == -1) {
      fprintf(stderr, "Error %d al abrir la memoria compartida del servidor "
                      "forked", errno);
      return -1;
  }

  // Asignar el tamaño de la memoria
  ftruncate(shmem_fd, sizeof(bool));

  // Mapear la memoria
  run = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED,
             shmem_fd, 0);

  if (run == MAP_FAILED)
  {
      fprintf(stderr, "Error %d al mapear la memoria compartida del servidor "
                      "forked\n ", errno);
      return -1;
  }

  printf("Inicializando servidor...\n");
  fd = tcp_connection_init(port_int, NULL, true);
  if (fd < 0) {
    fprintf(stderr, "Error inicializando servidor\n");
    return -1;
  }

  // Indicar a los procesos que el servidor está corriendo
  *run = true;

  // Unmap memory
  munmap(run, sizeof(bool));

  fprintf(stderr, "Servidor inicializado\n");

  return fd;
}

int execute_forked_server(int socket_file_descriptor, char *root){
    // struct sockaddr_in is defined in in.h
    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(client_address);
    int accept_response;
    int fork_response;
    int shmem_fd;
    bool *run;

    // Abrir memoria compartida en modo read/write
    shmem_fd = shm_open("forked_shmem", O_RDWR, S_IRUSR | S_IWUSR);

    if (shmem_fd == -1) {
        fprintf(stderr, "Error %d al abrir la memoria compartida del servidor "
                        "forked", errno);
        return errno;
    }

    // Asignar el tamaño de la memoria
    ftruncate(shmem_fd, sizeof(bool));

    // Mapear la memoria
    run = mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE, MAP_SHARED,
               shmem_fd, 0);

    if (run == MAP_FAILED)
    {
        fprintf(stderr, "Error %d al mapear la memoria compartida del servidor "
                        "forked\n ", errno);
        return errno;
    }

    int status;
    while (*run) {
        printf("Esperando solicitud...\n");
        accept_response = accept(socket_file_descriptor, (struct sockaddr*)&client_address, &client_address_length);
        printf("Solicitud recibida\n");
        if (accept_response < 0) {
            fprintf(stderr, "Error esperando una conexión de socket en la función accept. (Errno. %s)\n",
                    strerror(errno));
            respond_internal_server_error(socket_file_descriptor, SERVER_NAME);
            continue;
        }

        fork_response = fork();
        if (fork_response == 0) {
            printf("Un nuevo proceso se ha creado para atender a %s. PID: %d\n",
                   inet_ntoa(client_address.sin_addr), getpid());
            respond_to_request(root, accept_response, SERVER_NAME);
            printf("Terminando proceso que atendió a %s con PID %d\n",
                    inet_ntoa(client_address.sin_addr), getpid());
            exit(EXIT_SUCCESS);
        }

        status = close(accept_response);
        if (status < 0) {
            fprintf(stderr, "Error en la función close. (Errno %d: %s)\n",
                    errno, strerror(errno));
        }

        if (fork_response < 0) {
            fprintf(stderr, "Error ejecutando la función fork. (Errno. %s)\n",
                    strerror(errno));
            respond_service_unavailable(socket_file_descriptor, SERVER_NAME);
            continue;
        }

        printf("Se creó el proceso con PID: %d\n", fork_response);
    }

    // Unmap memory
    munmap(run, sizeof(bool));

    return EXIT_SUCCESS;
}
