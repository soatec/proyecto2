#include <stdio.h>            // input/output library
#include <unistd.h>           // misc. UNIX functions
#include <stdlib.h>           // standard library
#include <string.h>           // string library

#include <sys/types.h>        // socket types plus various type definitions
#include <sys/socket.h>       // socket definitions
#include <arpa/inet.h>        // inet (3) funtions
#include <fcntl.h>            // for O_* constants
#include <errno.h>            // error number library
#include <sys/sendfile.h>     // more constants
#include <sys/stat.h>         // more constants
#include <math.h>

#include <signal.h>           // signal handling
#include <sys/mman.h>         // mmap library

#include "preforked.h"
#include "utils.h"


typedef struct {
    char *ext;
    char *mediatype;
} extn;

//Possible media types
extn exts[] ={
    {"gif", "image/gif" },
    {"txt", "text/plain" },
    {"jpg", "image/jpg" },
    {"jpeg","image/jpeg"},
    {"png", "image/png" },
    {"ico", "image/ico" },
    {"zip", "image/zip" },
    {"gz",  "image/gz"  },
    {"tar", "image/tar" },
    {"htm", "text/html" },
    {"html","text/html" },
    {"pdf", "application/pdf"},
    {"zip", "application/octet-stream"},
    {"rar", "application/octet-stream"},
    {0,0} };

//#############################################################################################

// get a message from the socket until a blank line is recieved
char *getMessage(int fd)
{
    // A file stream
    FILE * sstream;

    // Try to open the socket to the file stream and handle any failures
    if ((sstream = fdopen(fd, "r")) == NULL)
    {
        fprintf(stderr, "Error opening file descriptor in getMessage()\n");
        exit(EXIT_FAILURE);
    }

    // Size variable for passing to getline
    size_t size = 1;

    char *block;

    // Allocate some memory for block and check it went ok
    if ((block = malloc(sizeof(char) *size)) == NULL)
    {
        fprintf(stderr, "Error allocating memory to block in getMessage\n");
        exit(EXIT_FAILURE);
    }

    // Set block to null *block = '\0';

    // Allocate some memory for tmp and check it went ok
    char *tmp;
    if ((tmp = malloc(sizeof(char) *size)) == NULL)
    {
        fprintf(stderr, "Error allocating memory to tmp in getMessage\n");
        exit(EXIT_FAILURE);
    }

    // Set tmp to null *tmp = '\0';

    // Int to keep track of what getline returns
    int end;
    // Int to help use resize block
    int oldsize = 1;

    // While getline is still getting data
    while ((end = getline(&tmp, &size, sstream)) > 0)
    {
        // If the line its read is a caridge return and a new line were at the end of the header so break
        if (strcmp(tmp, "\r\n") == 0)
        {
            break;
        }

        // Resize block
        block = realloc(block, size + oldsize);
        // Set the value of oldsize to the current size of block
        oldsize += size;
        // Append the latest line we got to block
        strcat(block, tmp);
    }

    // Free tmp a we no longer need it
    free(tmp);

    // Return the header
    return block;
}

int sendMessage(int fd, char *msg)
{
    return write(fd, msg, strlen(msg));
}

char *getFileName(char *msg, char *rootdir)
{
    // Allocate some memory for the filename and check it went OK
    char *file;
    if ((file = malloc(sizeof(char) *strlen(msg))) == NULL)
    {
        fprintf(stderr, "Error allocating memory to file in getFileName()\n");
        exit(EXIT_FAILURE);
    }

    // Get the filename from the header
    sscanf(msg, "GET %s HTTP/1.1", file);

    // Allocate some memory not in read only space to store "public_html"
    char *base;
    if ((base = malloc(sizeof(char) *(strlen(file) + 18))) == NULL)
    {
        fprintf(stderr, "Error allocating memory to base in getFileName()\n");
        exit(EXIT_FAILURE);
    }

    // Copy public to the non read only memory
    strcpy(base, rootdir);

    // Append the filename after public
    strcat(base, file);

    // Free file as we now have the file name in base
    free(file);

    // Return public/file.extension
    return base;
}

char *getContentType(char *msg)
{
    // Allocate some memory for the filename and check it went OK
    char *file;
    if ((file = malloc(sizeof(char) *strlen(msg))) == NULL)
    {
        fprintf(stderr, "Error allocating memory to file in getFileName()\n");
        exit(EXIT_FAILURE);
    }

    // Get the filename from the header
    sscanf(msg, "GET %s HTTP/1.1", file);

    // Allocate some memory not in read only space to store "public_html"
    char *base;
    if ((base = malloc(sizeof(char) *(strlen(file) + 18))) == NULL)
    {
        fprintf(stderr, "Error allocating memory to base in getFileName()\n");
        exit(EXIT_FAILURE);
    }

    char *ext = strchr(file, '.');
    if (ext != NULL)
    {
        int i;
        for (i = 0; exts[i].ext != NULL; i++)
        {
            if (strcmp(ext + 1, exts[i].ext) == 0)
            {
                strcpy(base, exts[i].mediatype);
                free(file);
                return base;
            }
        }
    }

    //Default value, as content type was invalid
    strcpy(base, "invalid");
    // Free file as we now have the file name in base
    free(file);

    // Return public_html/filetheywant.html
    return base;
}

int getFileSize(char *filename)
{
    // Get the size of this file for printing out later on
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

// parse a HTTP request and return an object with return code and filename
httpRequest parseRequest(char *msg, char *rootdir)
{
    httpRequest ret;

    // Allocate some memory to filename and check it goes OK
    char *filename;
    if ((filename = malloc(sizeof(char) *strlen(msg))) == NULL)
    {
        fprintf(stderr, "Error allocating memory to filename in parseRequest()\n");
        exit(EXIT_FAILURE);
    }

    // Find out what page they want
    filename = getFileName(msg, rootdir);

    // Allocate some memory to filename and check it goes OK
    char *contentType;
    if ((contentType = malloc(sizeof(char) *strlen(msg))) == NULL)
    {
        fprintf(stderr, "Error allocating memory to contentType in parseRequest()\n");
        exit(EXIT_FAILURE);
    }

    // Find out what page they want
    contentType = getContentType(msg);

    // Check if its a directory traversal attack
    char *badstring = "..";
    char *is_traversal = strstr(filename, badstring);
    is_traversal = NULL;    //Disable traversal check

    // Check if they asked for rootdir/ and give them index.html
    char *rootdirslash = malloc(strlen(rootdir) + 1);
    strcpy(rootdirslash, rootdir);
    strcat(rootdirslash, "/");
    double is_index = fmin((double) strcmp(filename, rootdir), (double) strcmp(filename, rootdirslash));
    free(rootdirslash);

    // Check if the page they want exists 
    FILE *exists = fopen(filename, "r");

    // If the badstring is found in the filename
    if (is_traversal != NULL)
    {
        // Return a 400 header and 400.html
        ret.returncode = 400;
        ret.filename = "400.html";
        ret.contentType = "text/html";
        fprintf(stderr, "[%i] 400 Bad Request (%s)\n", getpid(), filename);
    }

    // If they asked for / return index.html
    else if (is_index == 0 || strcmp(filename, rootdir) == 0)
    {
        //this catches index.html but return 404 as we dont have one
        ret.returncode = 404;   //200
        ret.filename = "404.html";  //index.html
        ret.contentType = "text/html";
        fprintf(stderr, "[%i] 404 Not Found (%s)\n", getpid(), filename);   
    }

    // If they asked for a specific page and it exists because we opened it sucessfully return it 
    else if (exists != NULL)
    {
        ret.returncode = 200;
        ret.filename = filename;
        ret.contentType = contentType;
        // Close the file stream
        fclose(exists);
    }

    // If we get here the file they want doesn't exist so return a 404
    else
    {
        ret.returncode = 404;
        ret.filename = "404.html";
        ret.contentType = "text/html";
        fprintf(stderr, "[%i] 404 Not Found (%s)\n", getpid(), filename);
    }

    // Return the structure containing the details
    return ret;
}

int writeFile(int fd, char *filename)
{
    if ((strcmp("404.html", filename) == 0) || (strcmp("400.html", filename) == 0))
    {
        return 0;
    }

    /*Open the file filename and echo the contents from it to the file descriptor fd */

    // Attempt to open the file 
    int read;
    if ((read = open(filename, O_RDONLY, 0)) < 0)
    {
        fprintf(stderr, "Error opening file in printFile()\n");
        exit(EXIT_FAILURE);
    }

    // Get the size of this file for printing out later on
    int totalsize;
    struct stat st;
    stat(filename, &st);
    totalsize = st.st_size;

    size_t total_bytes_sent = 0;
    ssize_t bytes_sent;
    while (total_bytes_sent < totalsize)
    {
        //Zero copy optimization
        if ((bytes_sent = sendfile(fd, read, 0, totalsize - total_bytes_sent)) <= 0)
        {
            if (errno == EINTR || errno == EAGAIN)
            {
                continue;
            }

            perror("Error on sendfile\n");
            return -1;
        }

        total_bytes_sent += bytes_sent;
    }

    // Return how big the file we sent out was
    return totalsize;
}

int writeHeader(int fd, int returncode, char *filename, char *contentType)
{
    char responseheader[120];
    int filesize = getFileSize(filename);

    // Print the header based on the return code
    switch (returncode)
    {
        case 200:
            snprintf(responseheader, sizeof(responseheader),
                "HTTP/1.1 200 OK\r\n"
                "Server: PreForkedServer\r\n"
                "Content-Length: %d\r\n"
                "Content-Type: %s\r\n\r\n", filesize, contentType);
            //fprintf(stderr, "Header = %s\n", responseheader);
            fprintf(stderr, "[%i] 200 OK (%s)\n", getpid(), filename);
            sendMessage(fd, responseheader);
            return strlen(responseheader);
            break;

        case 400:
            snprintf(responseheader, sizeof(responseheader),
                "HTTP/1.1 400 Bad Request\r\n"
                "Server: PreForkedServer\r\n"
                "Content-Length: %d\r\n"
                "Content-Type: text/html\r\n\r\n", filesize);
            sendMessage(fd, responseheader);
            return strlen(responseheader);
            break;

        case 404:
            snprintf(responseheader, sizeof(responseheader),
                "HTTP/1.1 404 Not Found\r\n"
                "Server: PreForkedServer\r\n"
                "Content-Length: %d\r\n"
                "Content-Type: text/html\r\n\r\n", filesize);
            sendMessage(fd, responseheader);
            return strlen(responseheader);
            break;
    }

    return 0;
}