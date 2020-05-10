#ifndef PROYECTO2_THREADED_H
#define PROYECTO2_THREADED_H

#include <pthread.h>
#include "utils.h"

// structure to hold the return code and the filepath to serve to client.
typedef struct {
    int returncode;
    char* filename;
    char* contentType;
} httpRequest;

// Structure to hold variables that will be placed in shared memory
typedef struct {
    pthread_mutex_t mutexlock;
    pthread_mutex_t accept_connection_lock;
    int totalbytes;
} sharedVariables;

typedef struct{
    int connfd_thread;
    char *root;
    sharedVariables sharedVars;
} threadInfo;


int getFileSize(char *filename);

// get a message from the socket until a blank line is recieved
char *getMessage(int fd);

// send a message to a socket file descripter
int sendMessage(int fd, char *msg);

// Extracts the filename needed from a GET request and adds public_html to the front of it
char* getFileName(char* msg, char* rootdir);

char* getContentType(char* msg);

// parse a HTTP request and return an object with return code and filename
httpRequest parseRequest(char* msg, char* rootdir);

// write a file out to a socket file descriptor
int writeFile(int fd, char *filename);

int writeHeader(int fd, int returncode, char* filename, char* contentType);

void* connection(void* p);


#endif //PROYECTO2_THREADED_H