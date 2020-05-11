#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include "secuencial.h"

#define NET_BUF_SIZE 10



int init_server(char *root, char *port){
	
	int listenfd; //listen socket
	int newfd;  // listen accept
    struct sockaddr_storage their_addr; // sock storage
    char net_buf[NET_BUF_SIZE]; 


    listenfd = get_listener_socket(root, port);

    if (listenfd < 0) {
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }

    printf("webserver: waiting for connections on port %s...\n", port);
    
    while(1) {
        socklen_t sin_size = sizeof their_addr;

        newfd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);
        if (newfd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            net_buf, sizeof net_buf);
        printf("server: got connection from %s\n", net_buf);

        handle_http_request(newfd);

        close(newfd);
    }

    return 0;
	 
}

int main(int argc, char *argv[]) {
  int   opt;
  char *root    = NULL;
  int   puerto  = -1;
  char *port = NULL;

  while ((opt = getopt(argc, argv, "p:r:")) != -1) {
      switch (opt) {
          case 'p':
              puerto = atoi(optarg);
              port = optarg;
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


  init_server(root, port);
  return EXIT_SUCCESS;
}
