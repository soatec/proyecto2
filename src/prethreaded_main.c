#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "prethreaded.h"

int main(int argc, char *argv[]) {
  int status;
  int   opt;
  char *root    = NULL;
  int   puerto  = -1;
  int   threads = -1;
  servidor_prethreaded_t servidor;

  while ((opt = getopt(argc, argv, "p:r:n:")) != -1) {
      switch (opt) {
          case 'p':
              // Rango de puertos válidos 1025-
              puerto = atoi(optarg);
              break;
          case 'r':
              root = optarg;
              break;
          case 'n':
              threads = atoi(optarg);
              break;
          default:
              fprintf(stderr, "Uso: %s -p puerto -r root_servidor "
                      "-n número_threads\n", argv[0]);
              return EXIT_FAILURE;
      }
  }

  if (root == NULL) {
      fprintf(stderr, "r root_servidor es un parámetro obligatorio\n");
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
      fprintf(stderr, "-n número_threads es un parámetro obligatorio\n");
      return EXIT_FAILURE;
  } else if (threads < 0) {
      fprintf(stderr, "-n número_threads debe ser positivo\n");
      return EXIT_FAILURE;
  }

  printf("\n\n  ______                                 __        __                            _______                        __      __                                            __                  __ \n");
  printf(" /      \\                               |  \\      |  \\                          |       \\                      |  \\    |  \\                                          |  \\                |  \\\n");
  printf("|  $$$$$$\\  ______    ______  __     __  \\$$  ____| $$  ______    ______        | $$$$$$$\\  ______    ______  _| $$_   | $$____    ______    ______    ______    ____| $$  ______    ____| $$\n");
  printf("| $$___\\$$ /      \\  /      \\|  \\   /  \\|  \\ /      $$ /      \\  /      \\       | $$__/ $$ /      \\  /      \\|   $$ \\  | $$    \\  /      \\  /      \\  |      \\  /      $$ /      \\  /      $$\n");
  printf(" \\$$    \\ |  $$$$$$\\|  $$$$$$\\\\$$\\ /  $$| $$|  $$$$$$$|  $$$$$$\\|  $$$$$$\\      | $$    $$|  $$$$$$\\|  $$$$$$\\\\$$$$$$  | $$$$$$$\\|  $$$$$$\\|  $$$$$$\\  \\$$$$$$\\|  $$$$$$$|  $$$$$$\\|  $$$$$$$\n");
  printf(" _\\$$$$$$\\| $$    $$| $$   \\$$ \\$$\\  $$ | $$| $$  | $$| $$  | $$| $$   \\$$      | $$$$$$$ | $$   \\$$| $$    $$ | $$ __ | $$  | $$| $$   \\$$| $$    $$ /      $$| $$  | $$| $$    $$| $$  | $$\n");
  printf("|  \\__| $$| $$$$$$$$| $$        \\$$ $$  | $$| $$__| $$| $$__/ $$| $$            | $$      | $$      | $$$$$$$$ | $$|  \\| $$  | $$| $$      | $$$$$$$$|  $$$$$$$| $$__| $$| $$$$$$$$| $$__| $$\n");
  printf(" \\$$    $$ \\$$     \\| $$         \\$$$   | $$ \\$$    $$ \\$$    $$| $$            | $$      | $$       \\$$     \\  \\$$  $$| $$  | $$| $$       \\$$     \\ \\$$    $$ \\$$    $$ \\$$     \\ \\$$    $$\n");
  printf("  \\$$$$$$   \\$$$$$$$ \\$$          \\$     \\$$  \\$$$$$$$  \\$$$$$$  \\$$             \\$$       \\$$        \\$$$$$$$   \\$$$$  \\$$   \\$$ \\$$        \\$$$$$$$  \\$$$$$$$  \\$$$$$$$  \\$$$$$$$  \\$$$$$$$\n\n");

  status = prethreaded_server_init((uint16_t)puerto, threads, &servidor);
  if(status) return status;

  printf("%s ejecutando con un máximo de %d threads con el root path %s "
         "en el puerto %d de la máquina IP %s\n",argv[0], threads, root, puerto,
          inet_ntoa(servidor.address.sin_addr));

  status = prethreaded_server_run(servidor);
  if (status) return status;

  fprintf(stdout, "%s\n", "Press any key to exit");
  getchar();

  status = prethreaded_server_uninit(servidor);
  if (status) return status;

  return EXIT_SUCCESS;
}
