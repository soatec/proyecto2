#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include "threaded.h"

int main(int argc, char *argv[]) {
    int   opt;
    int   puerto  = -1;
    char *root    = NULL;
    DIR   *dir;

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

    printf("\n\n  ______                                 __        __                            ________  __                                            __                  __\n");
    printf(" /      \\                               |  \\      |  \\                          |        \\|  \\                                          |  \\                |  \\\n");
    printf("|  $$$$$$\\  ______    ______  __     __  \\$$  ____| $$  ______    ______         \\$$$$$$$$| $$____    ______    ______    ______    ____| $$  ______    ____| $$\n");
    printf("| $$___\\$$ /      \\  /      \\|  \\   /  \\|  \\ /      $$ /      \\  /      \\          | $$   | $$    \\  /      \\  /      \\  |      \\  /      $$ /      \\  /      $$\n");
    printf(" \\$$    \\ |  $$$$$$\\|  $$$$$$\\\\$$\\ /  $$| $$|  $$$$$$$|  $$$$$$\\|  $$$$$$\\         | $$   | $$$$$$$\\|  $$$$$$\\|  $$$$$$\\  \\$$$$$$\\|  $$$$$$$|  $$$$$$\\|  $$$$$$$\n");
    printf(" _\\$$$$$$\\| $$    $$| $$   \\$$ \\$$\\  $$ | $$| $$  | $$| $$  | $$| $$   \\$$         | $$   | $$  | $$| $$   \\$$| $$    $$ /      $$| $$  | $$| $$    $$| $$  | $$\n");
    printf("|  \\__| $$| $$$$$$$$| $$        \\$$ $$  | $$| $$__| $$| $$__/ $$| $$               | $$   | $$  | $$| $$      | $$$$$$$$|  $$$$$$$| $$__| $$| $$$$$$$$| $$__| $$\n");
    printf(" \\$$    $$ \\$$     \\| $$         \\$$$   | $$ \\$$    $$ \\$$    $$| $$               | $$   | $$  | $$| $$       \\$$     \\ \\$$    $$ \\$$    $$ \\$$     \\ \\$$    $$\n");
    printf("  \\$$$$$$   \\$$$$$$$ \\$$          \\$     \\$$  \\$$$$$$$  \\$$$$$$  \\$$                \\$$    \\$$   \\$$ \\$$        \\$$$$$$$  \\$$$$$$$  \\$$$$$$$  \\$$$$$$$  \\$$$$$$$\n\n");

    printf("%s ejecutando con el root path %s en el puerto %d\n",argv[0], root,
           puerto);
    execute_threaded_server(puerto, root);
    return EXIT_SUCCESS;
}