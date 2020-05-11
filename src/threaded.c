#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include "threaded.h"
#include "utils.h"

#define SERVER_NAME "ThreadedServer"

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

void cleanup(int sig) {
    printf("Cleaning up connections and exiting.\n");
    tcp_connection_uninit(listening_socket);
    exit(EXIT_SUCCESS);
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
    socklen_t addr_size = sizeof(servaddr);

    (void) signal(SIGINT, cleanup);

    listening_socket = tcp_connection_init(port_int, NULL, true);
    if (listening_socket < 0) {
      return EXIT_FAILURE;
    }

    shared_variables = malloc(sizeof(shared_variables_t));
    pthread_mutex_init(&shared_variables->total_bytes_mutex_lock, NULL);
    shared_variables->total_bytes = 0;

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
