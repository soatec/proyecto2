#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/mman.h>
#include <pthread.h>
#include "threaded.h"
#include "utils.h"

#define SERVER_BACKLOG 100
#define SERVER_NAME "ThreadedServer"


// Structure to hold variables that will be placed in shared memory
typedef struct {
    pthread_mutex_t total_bytes_mutex_lock;
    int             total_bytes;
} shared_variables_t;

typedef struct{
    int                file_descriptor;
    char               *root;
    shared_variables_t *shared_variables;
} thread_info_t;

int listening_socket;

int save_global_count_bytes(int bytes_sent, shared_variables_t *shared_variables) {
    pthread_mutex_lock(&shared_variables->total_bytes_mutex_lock);
    (*shared_variables).total_bytes += bytes_sent;
    pthread_mutex_unlock(&shared_variables->total_bytes_mutex_lock);
    return (*shared_variables).total_bytes;
}

// clean up listening socket on ctrl-c
void cleanup(int sig)
{
    printf("PID:%i Cleaning up connections and exiting.\n", getpid());

    // try to close the listening socket
    if (close(listening_socket) < 0)
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

void* process_request(void *thread_info_void){
    int response_size;
    int total_data;
    thread_info_t *thread_info = (thread_info_t*)thread_info_void;
    response_size = respond_to_request(thread_info->root, thread_info->file_descriptor, SERVER_NAME);
    total_data = save_global_count_bytes(response_size, thread_info->shared_variables);
    printf("Global count of data sent out: %d bytes", total_data);
    pthread_exit(NULL);
}

int execute_threaded_server(int port_int, char *root) {
    struct sockaddr_in servaddr;
    shared_variables_t *shared_variables;
    short int port = port_int;
    socklen_t addr_size = sizeof(servaddr);

    // set up signal handler for ctrl-c
    (void) signal(SIGINT, cleanup);

    // create the listening socket
    if ((listening_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Error creating listening socket.\n");
        exit(EXIT_FAILURE);
    }

    // set all bytes in socket address structure to zero, and fill in the relevant data members
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listening_socket, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        fprintf(stderr, "Error calling bind()\n");
        exit(EXIT_FAILURE);
    }

    shared_variables = malloc(sizeof(shared_variables_t));
    pthread_mutex_init(&shared_variables->total_bytes_mutex_lock, NULL);
    shared_variables->total_bytes = 0;

    if ((listen(listening_socket, SERVER_BACKLOG)) == -1) {
        fprintf(stderr, "Error Listening\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        thread_info_t current_thread_info;
        current_thread_info.root = malloc(strlen(root));
        strcpy(current_thread_info.root, root);
        printf("Waiting for a request \n");
        pthread_t current_thread;
        current_thread_info.file_descriptor =  accept(listening_socket, (struct sockaddr *) &servaddr, &addr_size);
        if (current_thread_info.file_descriptor < 0){
            fprintf(stderr, "Error accepting connection \n");
            return EXIT_FAILURE;
        }
        current_thread_info.shared_variables = shared_variables;
        pthread_create(&current_thread, NULL, process_request, &current_thread_info);
        pthread_join(current_thread, NULL);
    }
    return EXIT_FAILURE;
}

