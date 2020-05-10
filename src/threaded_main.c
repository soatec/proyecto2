#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <dirent.h>

#include <sys/types.h>        // socket types plus various type definitions
#include <sys/socket.h>       // socket definitions
#include <arpa/inet.h>        // inet (3) funtions
#include <fcntl.h>            // for O_* constants
#include <errno.h>
#include <sys/sendfile.h>     // more constants
#include <sys/stat.h>         // more constants

#include <signal.h>           // signal handling
#include <sys/mman.h>         // mmap library
#include <pthread.h>
#include <sys/wait.h>

#include "threaded.h"


#define SERVER_BACKLOG 100


// Structure to hold variables that will be placed in shared memory
typedef struct {
    pthread_mutex_t mutexlock;
    pthread_mutex_t accept_connection_lock;
    int totalbytes;
} sharedVariables;

typedef struct{
    int connfd_thread;
    char *root;
} threadInfo;

int list_s;

// Increment the global count of data sent out 
int recordTotalBytes(int bytes_sent, sharedVariables *mempointer)
{
    // Lock the mutex
    pthread_mutex_lock(&(*mempointer).mutexlock);
    // Increment bytes_sent
    (*mempointer).totalbytes += bytes_sent;
    // Unlock the mutex
    pthread_mutex_unlock(&(*mempointer).mutexlock);
    // Return the new byte count
    return (*mempointer).totalbytes;
}

// clean up listening socket on ctrl-c
void cleanup(int sig)
{
    printf("PID:%i Cleaning up connections and exiting.\n", getpid());

    // try to close the listening socket
    if (close(list_s) < 0)
    {
        fprintf(stderr, "Error calling close()\n");
        exit(EXIT_FAILURE);
    }

    // Close the shared memory we used
    shm_unlink("/sharedmem");

    // exit with success
    exit(EXIT_SUCCESS);
}

int my_get_char(void)
{
    int ch;
    struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

void* connection(void *p){



    // Sizes of data were sending out
    int headersize;
    int pagesize;

    threadInfo *info = (threadInfo*)p;

    int connfd_thread = info->connfd_thread;

    char *header = getMessage(connfd_thread);

    httpRequest details = parseRequest(header, info->root);

    free(header);

    headersize = writeHeader(connfd_thread, details.returncode, details.filename, details.contentType);

    pagesize = writeFile(connfd_thread, details.filename);

    printf("[%d] served a request of %d bytes\n", getpid(), headersize + pagesize);

    close(connfd_thread);
    pthread_exit(NULL);

}

int main(int argc, char *argv[]) {
  int   opt;
  char *root    = NULL;
  int   puerto  = -1;

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

   DIR * dir;
    if ((dir = opendir(root)))
    {
        closedir(dir);
    }
    else if (ENOENT == errno)
    {
        fprintf(stderr, "-r root_servidor no es un directorio valido\n");
        return EXIT_FAILURE;
    }
    else
    {
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

    short int port = puerto;  //  port number
    struct sockaddr_in servaddr;  //  socket address structure

    // set up signal handler for ctrl-c
    (void) signal(SIGINT, cleanup);

    // create the listening socket
    if ((list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Error creating listening socket.\n");
        exit(EXIT_FAILURE);
    }

    // set all bytes in socket address structure to zero, and fill in the relevant data members
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // bind to the socket address

    if (bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        fprintf(stderr, "Error calling bind()\n");
        exit(EXIT_FAILURE);
    }

    // Set up some shared memory to store our shared variables in

    // Close the shared memory we use just to be safe
    shm_unlink("/sharedmem");

    int sharedmem;

    // Open the memory
    if ((sharedmem = shm_open("/sharedmem", O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1)
    {
        fprintf(stderr, "Error opening sharedmem in main() errno is: %s ", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Set the size of the shared memory to the size of my structure
    ftruncate(sharedmem, sizeof(sharedVariables));

    // Map the shared memory into our address space
    sharedVariables * mempointer;

    // Set mempointer to point at the shared memory
    mempointer = mmap(NULL, sizeof(sharedVariables), PROT_READ | PROT_WRITE, MAP_SHARED, sharedmem, 0);

    // Check the memory allocation went OK
    if (mempointer == MAP_FAILED)
    {
        fprintf(stderr, "Error setting shared memory for sharedVariables in recordTotalBytes() error is %d \n ", errno);
        exit(EXIT_FAILURE);
    }

    // Initalise the mutex
    pthread_mutex_init(&(*mempointer).mutexlock, NULL);
    pthread_mutex_init(&(*mempointer).accept_connection_lock, NULL);
    // Set total bytes sent to 0
    (*mempointer).totalbytes = 0;

    // Size of the address
    socklen_t addr_size = sizeof(servaddr);

    threadInfo connfd[100];  //  connection socket
    pthread_t threads[100];
    int thread_count = 0;

    while(thread_count < 100){
        // Listen on socket list_s
        if ((listen(list_s, SERVER_BACKLOG)) == -1)
        {
            fprintf(stderr, "Error Listening\n");
            exit(EXIT_FAILURE);
        }
        connfd[thread_count].root = malloc(strlen(root));
        strcpy(connfd[thread_count].root, root); 
        printf("Waiting for a request \n");

        pthread_mutex_lock(&(*mempointer).accept_connection_lock);
        connfd[thread_count].connfd_thread =  accept(list_s, (struct sockaddr *) &servaddr, &addr_size);
        pthread_mutex_unlock(&(*mempointer).accept_connection_lock);

        if(connfd[thread_count].connfd_thread < 0){
            fprintf(stderr, "Error accepting connection \n");
            exit(1);
        }

        //printf("%d \n", connfd[thread_count].connfd_thread);
        //printf("%s \n", connfd[thread_count].root);

        pthread_create(&threads[thread_count], NULL, connection, &connfd[thread_count]);
        pthread_join(threads[thread_count], NULL);
        thread_count++;
    }

  return EXIT_SUCCESS;
}
