#ifndef PROYECTO2_PREFORKED_H
#define PROYECTO2_PREFORKED_H

#include "utils.h"

// structure to hold the return code and the filepath to serve to client.
typedef struct {
    int returncode;
    char* filename;
    char* contentType;
} httpRequest;

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

// print a file out to a socket file descriptor
int printFile(int fd, char *filename);

int printHeader(int fd, int returncode, char* filename, char* contentType);

#endif //PROYECTO2_PREFORKED_H