#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <dirent.h>
#include "prethreaded.h"

static servidor_prethreaded_t servidor;

void sigint_handler(int sig_num) {
    printf("CTRL+C fue presionado, el servidor debe terminar\n");
    prethreaded_server_uninit(&servidor);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    int status;
    int   opt;
    char *root    = NULL;
    int   puerto  = -1;
    int   threads = -1;
    DIR  *dir;

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
           "en el puerto %d\n",argv[0], threads, root, puerto);

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        fprintf(stderr, "Error %d al instalar el handler para SIGINT\n", errno);
        return EXIT_FAILURE;
    }

    printf("Presione CTRL+C para terminar el programa\n");

    status = prethreaded_server_run(&servidor);
    return status;
}
