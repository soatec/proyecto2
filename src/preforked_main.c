#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "preforked.h"

int main(int argc, char *argv[]) {
  int   opt;
  char *root     = NULL;
  int   puerto   = -1;
  int   procesos = -1;

  while ((opt = getopt(argc, argv, "p:r:n:")) != -1) {
      switch (opt) {
          case 'p':
              puerto = atoi(optarg);
              break;
          case 'r':
              root = optarg;
              break;
          case 'n':
              procesos = atoi(optarg);
              break;
          default:
              fprintf(stderr, "Uso: %s -p puerto -r root_servidor "
                      "-n número_procesos\n", argv[0]);
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

  if (procesos == -1) {
      fprintf(stderr, "-n número_procesos es un parámetro obligatorio\n");
      return EXIT_FAILURE;
  } else if (procesos < 0) {
      fprintf(stderr, "-n número_procesos debe ser positivo\n");
      return EXIT_FAILURE;
  }

  printf("\n\n  ______                                 __        __                            _______                        ______                      __                        __\n");
  printf(" /      \\                               |  \\      |  \\                          |       \\                      /      \\                    |  \\                      |  \\\n");
  printf("|  $$$$$$\\  ______    ______  __     __  \\$$  ____| $$  ______    ______        | $$$$$$$\\  ______    ______  |  $$$$$$\\ ______    ______  | $$   __   ______    ____| $$\n");
  printf("| $$___\\$$ /      \\  /      \\|  \\   /  \\|  \\ /      $$ /      \\  /      \\       | $$__/ $$ /      \\  /      \\ | $$_  \\$$/      \\  /      \\ | $$  /  \\ /      \\  /      $$\n");
  printf(" \\$$    \\ |  $$$$$$\\|  $$$$$$\\\\$$\\ /  $$| $$|  $$$$$$$|  $$$$$$\\|  $$$$$$\\      | $$    $$|  $$$$$$\\|  $$$$$$\\| $$ \\   |  $$$$$$\\|  $$$$$$\\| $$_/  $$|  $$$$$$\\|  $$$$$$$\n");
  printf(" _\\$$$$$$\\| $$    $$| $$   \\$$ \\$$\\  $$ | $$| $$  | $$| $$  | $$| $$   \\$$      | $$$$$$$ | $$   \\$$| $$    $$| $$$$   | $$  | $$| $$   \\$$| $$   $$ | $$    $$| $$  | $$\n");
  printf("|  \\__| $$| $$$$$$$$| $$        \\$$ $$  | $$| $$__| $$| $$__/ $$| $$            | $$      | $$      | $$$$$$$$| $$     | $$__/ $$| $$      | $$$$$$\\ | $$$$$$$$| $$__| $$\n");
  printf(" \\$$    $$ \\$$     \\| $$         \\$$$   | $$ \\$$    $$ \\$$    $$| $$            | $$      | $$       \\$$     \\| $$      \\$$    $$| $$      | $$  \\$$\\ \\$$     \\ \\$$    $$\n");
  printf("  \\$$$$$$   \\$$$$$$$ \\$$          \\$     \\$$  \\$$$$$$$  \\$$$$$$  \\$$             \\$$       \\$$        \\$$$$$$$ \\$$       \\$$$$$$  \\$$       \\$$   \\$$  \\$$$$$$$  \\$$$$$$$\n\n");

  printf("%s ejecutando con un máximo de %d procesos con el root path %s "
         "en el puerto %d\n",argv[0], procesos, root, puerto);

  return EXIT_SUCCESS;
}
