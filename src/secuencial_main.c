#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "secuencial.h"

int main(int argc, char *argv[]) {
    int   opt;
    char *root    = NULL;
    int   puerto  = -1;
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

    printf("\n\n  ______                                 __        __                             ______                                                               __            __\n");
    printf(" /      \\                               |  \\      |  \\                           /      \\                                                             |  \\          |  \n");
    printf("|  $$$$$$\\  ______    ______  __     __  \\$$  ____| $$  ______    ______        |  $$$$$$\\  ______    _______  __    __   ______   _______    _______  \\$$  ______  | $$\n");
    printf("| $$___\\$$ /      \\  /      \\|  \\   /  \\|  \\ /      $$ /      \\  /      \\       | $$___\\$$ /      \\  /       \\|  \\  |  \\ /      \\ |       \\  /       \\|  \\ |      \\ | $$\n");
    printf(" \\$$    \\ |  $$$$$$\\|  $$$$$$\\\\$$\\ /  $$| $$|  $$$$$$$|  $$$$$$\\|  $$$$$$\\       \\$$    \\ |  $$$$$$\\|  $$$$$$$| $$  | $$|  $$$$$$\\| $$$$$$$\\|  $$$$$$$| $$  \\$$$$$$\\| $$\n");
    printf(" _\\$$$$$$\\| $$    $$| $$   \\$$ \\$$\\  $$ | $$| $$  | $$| $$  | $$| $$   \\$$       _\\$$$$$$\\| $$    $$| $$      | $$  | $$| $$    $$| $$  | $$| $$      | $$ /      $$| $$\n");
    printf("|  \\__| $$| $$$$$$$$| $$        \\$$ $$  | $$| $$__| $$| $$__/ $$| $$            |  \\__| $$| $$$$$$$$| $$_____ | $$__/ $$| $$$$$$$$| $$  | $$| $$_____ | $$|  $$$$$$$| $$\n");
    printf(" \\$$    $$ \\$$     \\| $$         \\$$$   | $$ \\$$    $$ \\$$    $$| $$             \\$$    $$ \\$$     \\ \\$$     \\ \\$$    $$ \\$$     \\| $$  | $$ \\$$     \\| $$ \\$$    $$| $$\n");
    printf("  \\$$$$$$   \\$$$$$$$ \\$$          \\$     \\$$  \\$$$$$$$  \\$$$$$$  \\$$              \\$$$$$$   \\$$$$$$$  \\$$$$$$$  \\$$$$$$   \\$$$$$$$ \\$$   \\$$  \\$$$$$$$ \\$$  \\$$$$$$$ \\$$\n\n");

    printf("%s ejecutando con el root path %s en el puerto %d\n",argv[0], root,
           puerto);

    server_status = execute_sequential_server(puerto, root);

    return server_status;
}
