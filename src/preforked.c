#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/mman.h>
#include <termios.h>
#include <pthread.h>
#include <sys/wait.h>
#include "preforked.h"
#include "utils.h"

#define SERVER_NAME "PreforkedServer"
#define SERVER_BACKLOG 100


// Structure to hold variables that will be placed in shared memory
typedef struct {
    pthread_mutex_t mutexlock;
    pthread_mutex_t accept_connection_lock;
    int totalbytes;
} shared_variables_t;

int list_s;                   // listening socket


int save_global_count_bytes(int bytes_sent, shared_variables_t *mempointer)
{
    pthread_mutex_lock(&(*mempointer).mutexlock);
    (*mempointer).totalbytes += bytes_sent;
    pthread_mutex_unlock(&(*mempointer).mutexlock);
    // Return the new byte count
    return (*mempointer).totalbytes;
}

// clean up listening socket on ctrl-c
void cleanup(int sig) {
    printf("PID:%i Cleaning up connections and exiting.\n", getpid());

    // Close listening socket
    tcp_connection_uninit(list_s);

    // Close the shared memory we used
    shm_unlink("/sharedmem");

    // exit with success
    exit(EXIT_SUCCESS);
}

int my_get_char(void) {
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

int execute_preforked_server(int puerto, char *root, int procesos) {
    int response_size;
    int conn_s;
    int totaldata;
    int children = 0;
    int childref[procesos - 1];
    int sharedmem;
    struct sockaddr_in servaddr;
    pid_t pid;

    // set up signal handler for ctrl-c
    (void) signal(SIGINT, cleanup);

    // Create listening socket
    list_s = tcp_connection_init(puerto, NULL);
    if (list_s < 0) {
      return EXIT_FAILURE;
    }

    // Close shared memory
    shm_unlink("/sharedmem");

    if ((sharedmem = shm_open("/sharedmem", O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1) {
        fprintf(stderr, "Error opening sharedmem in main() errno is: %s ", strerror(errno));
        return EXIT_FAILURE;
    }
    ftruncate(sharedmem, sizeof(shared_variables_t));
    shared_variables_t * mempointer;
    mempointer = mmap(NULL, sizeof(shared_variables_t), PROT_READ | PROT_WRITE, MAP_SHARED, sharedmem, 0);

    if (mempointer == MAP_FAILED) {
        fprintf(stderr, "Error setting shared memory for shared_variables_t in save_global_count_bytes() error is %d \n ", errno);
        return EXIT_FAILURE;
    }
    pthread_mutex_init(&(*mempointer).mutexlock, NULL);
    pthread_mutex_init(&(*mempointer).accept_connection_lock, NULL);
    (*mempointer).totalbytes = 0;
    socklen_t addr_size = sizeof(servaddr);

    while (1) {
        if (children == procesos) {
            printf("Press any key to close server.\n");
            my_get_char();
            printf("Server Closing...\n");
            int i;
            for (i = 0; i < children; i++) {
                kill(childref[i], 2);
            }
            wait(NULL);
            totaldata = save_global_count_bytes(0, mempointer);
            fprintf(stderr,"Total bytes sent %d\n", totaldata);
            fprintf(stderr, "Parent ");
            cleanup(2);
            break;
        }

        if (children < procesos)  {
            pid = fork();
            childref[children] = pid;
            children++;
        }

        if (pid == -1) {
            fprintf(stderr, "can't fork, error %d\n", errno);
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            while (1) {
                pthread_mutex_lock(&(*mempointer).accept_connection_lock);
                conn_s = accept(list_s, (struct sockaddr *) &servaddr, &addr_size);
                pthread_mutex_unlock(&(*mempointer).accept_connection_lock);

                if (conn_s == -1) {
                    fprintf(stderr, "Error accepting connection \n");
                    exit(EXIT_FAILURE);
                }
                response_size = respond_to_request(root, conn_s, SERVER_NAME);
                totaldata = save_global_count_bytes(response_size, mempointer);
                printf("Global count of data sent out: %d bytes", totaldata);
            }
            exit(EXIT_SUCCESS);
        }
    }
    return EXIT_FAILURE;
}
