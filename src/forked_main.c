#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dirent.h>
#include "forked.h"

static int socket_fd;

void sigint_handler(int sig_num) {
  int shmem_fd;
  bool *run;

  // Abrir memoria compartida en modo read/write
  shmem_fd = shm_open("/forked_shmem", O_RDWR, S_IRUSR | S_IWUSR);

  if (shmem_fd == -1) {
      fprintf(stderr, "Error %d al abrir la memoria compartida del servidor "
                      "forked", errno);
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
  }

  // Indicar a los procesos que deben terminar
  *run = false;

  // Unmap memory and close file
  munmap(run, sizeof(bool));
  shm_unlink("forked_shmem");

  printf("CTRL+C fue presionado, el servidor debe terminar\n");
  tcp_connection_uninit(socket_fd);
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    int   opt;
    char *root   = NULL;
    int   puerto = -1;
    int   server_status;
    DIR  *dir;

    while ((opt = getopt(argc, argv, "p:r:")) != -1) {
        switch (opt) {
            case 'p':
                puerto = atoi(optarg);
                break;
            case 'r':
                root = optarg;
                break;
            default:
                fprintf(stderr, "Uso: %s -p puerto -r root_servidor\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (root == NULL) {
        fprintf(stderr, "r root_servidor es un parámetro obligatorio\n");
        return EXIT_FAILURE;
    }
    dir = opendir(root);
    if (dir) {
        closedir(dir);
    } else if (ENOENT == errno) {
        fprintf(stderr, "-r root_servidor no es un directorio valido\n");
        return EXIT_FAILURE;
    } else {
        fprintf(stderr, "-r root_servidor no es un directorio valido\n");
        return EXIT_FAILURE;
    }

    if (puerto == -1) {
        fprintf(stderr, "-p puerto es un parámetro obligatorio\n");
        return EXIT_FAILURE;
    } else if (puerto < 0) {
        fprintf(stderr, "-p puerto debe ser positivo\n");
        return EXIT_FAILURE;
    }

    printf("\n\n  ______                                 __        __                            ________                    __                        __\n");
    printf(" /      \\                               |  \\      |  \\                          |        \\                  |  \\                      |  \\\n");
    printf("|  $$$$$$\\  ______    ______  __     __  \\$$  ____| $$  ______    ______        | $$$$$$$$______    ______  | $$   __   ______    ____| $$\n");
    printf("| $$___\\$$ /      \\  /      \\|  \\   /  \\|  \\ /      $$ /      \\  /      \\       | $$__   /      \\  /      \\ | $$  /  \\ /      \\  /      $$\n");
    printf(" \\$$    \\ |  $$$$$$\\|  $$$$$$\\\\$$\\ /  $$| $$|  $$$$$$$|  $$$$$$\\|  $$$$$$\\      | $$  \\ |  $$$$$$\\|  $$$$$$\\| $$_/  $$|  $$$$$$\\|  $$$$$$$\n");
    printf(" _\\$$$$$$\\| $$    $$| $$   \\$$ \\$$\\  $$ | $$| $$  | $$| $$  | $$| $$   \\$$      | $$$$$ | $$  | $$| $$   \\$$| $$   $$ | $$    $$| $$  | $$\n");
    printf("|  \\__| $$| $$$$$$$$| $$        \\$$ $$  | $$| $$__| $$| $$__/ $$| $$            | $$    | $$__/ $$| $$      | $$$$$$\\ | $$$$$$$$| $$__| $$\n");
    printf(" \\$$    $$ \\$$     \\| $$         \\$$$   | $$ \\$$    $$ \\$$    $$| $$            | $$     \\$$    $$| $$      | $$  \\$$\\ \\$$     \\ \\$$    $$\n");
    printf("  \\$$$$$$   \\$$$$$$$ \\$$          \\$     \\$$  \\$$$$$$$  \\$$$$$$  \\$$             \\$$      \\$$$$$$  \\$$       \\$$   \\$$  \\$$$$$$$  \\$$$$$$$\n");


    printf("%s ejecutando con el root path %s en el puerto %d\n",argv[0], root,
           puerto);

    socket_fd = init_forked_server(puerto);
    if (socket_fd < 0) {
      return EXIT_FAILURE;
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
      fprintf(stderr, "Error %d al instalar el handler para SIGINT\n", errno);
      return EXIT_FAILURE;
    }

    printf("Presione CTRL+C para terminar el programa\n");

    server_status = execute_forked_server(socket_fd, root);

    return server_status;
}
