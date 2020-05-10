#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include "utils.h"

#define WRITE_BUFFER_SIZE 1024
#define READ_BUFFER_SIZE 212992
#define MEDIA_TYPE_MAX_SIZE 25

typedef struct {
    char *extension;
    char *media_type;
} extension_t;

extension_t extensions[] ={
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
        {NULL,NULL}
};

void close_connection(int file_descriptor) {
    // SHUT_WR = No more transmissions
    shutdown(file_descriptor, SHUT_RDWR);
    close(file_descriptor);
}


int get_file_size(char *filename) {
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

char *get_content_type(char *file_path) {
    char *content_type;
    content_type = malloc(sizeof(char) * MEDIA_TYPE_MAX_SIZE);
    if (content_type == NULL) {
        fprintf(stderr, "Error allocating memory for content_type\n");
        exit(EXIT_FAILURE);
    }

    char *file_extension = strchr(file_path, '.') + 1;
    if (file_extension == NULL) {
        strcpy(content_type, "invalid");
        return content_type;
    }

    for (int i = 0; extensions[i].extension != NULL; i++) {
        if (strcmp(file_extension, extensions[i].extension) == 0) {
            strcpy(content_type, extensions[i].media_type);
            break;
        } else {
            strcpy(content_type, "invalid");
        }
    }
    return content_type;
}

int write_header(int file_descriptor, int return_code, char *file_name, char *server_name){
    char response_header[120];
    int file_size = 0;
    switch (return_code) {
        case 200:
            file_size = get_file_size(file_name);
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 200 OK\r\n"
                     "Server: %s\r\n"
                     "Content-Length: %d\r\n"
                     "Content-Type: %s\r\n\r\n", server_name, file_size, get_content_type(file_name));
            break;

        case 400:
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 400 Bad Request\r\n"
                     "Server: %s\r\n"
                     "Content-Length: 0\r\n"
                     "Content-Type: text/html\r\n\r\n", server_name);
            break;

        case 404:
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 404 Not Found\r\n"
                     "Server: %s\r\n"
                     "Content-Length: 0\r\n"
                     "Content-Type: text/html\r\n\r\n", server_name);
            break;
        default:
        case 501:
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 501 Not Implemented\r\n"
                     "Server: %s\r\n"
                     "Content-Length: 0\r\n"
                     "Content-Type: text/html\r\n\r\n", server_name);
    }
    write(file_descriptor, response_header, strlen(response_header));
    printf("Header enviado:\n\n%sFin del header\n", response_header);
    return (int)strlen(response_header) + file_size;
}

int respond_to_get_request(char *root, int file_descriptor,
        char *requested_resource_path, char *server_name) {
    int response_size = 0;
    char path[PATH_MAX];
    int requested_file_descriptor;
    int bytes_read;
    char data_to_send[WRITE_BUFFER_SIZE];

    if (strncmp(requested_resource_path, "/\0", 2) == 0) {
        requested_resource_path = "/index.html";
    }
    strcpy(path, root);
    strcpy(&path[strlen(root)], requested_resource_path);
    printf("Archivo: %s\n", path);
    requested_file_descriptor = open(path, O_RDONLY);
    if (requested_file_descriptor != -1) {
        printf("Archivo encontrado\n");
        response_size = write_header(file_descriptor, 200, path, server_name);
        while ((bytes_read=read(requested_file_descriptor, data_to_send, WRITE_BUFFER_SIZE)) > 0){
            if (bytes_read < 0) {
                fprintf(stderr, "Error en la función read. (Errno %d: %s)\n",
                        errno, strerror(errno));
                close_connection(file_descriptor);
            }
            write(file_descriptor, data_to_send, bytes_read);
        }
    } else {
        printf("Archivo no encontrado\n");
        response_size = write_header(file_descriptor, 404, NULL, server_name);
    }
    return response_size;
}

void respond_to_put_request(char *root, int file_descriptor,
        char *requested_resource_path, char *server_name) {

}

void respond_to_delete_request(char *root, int file_descriptor,
        char *requested_resource_path, char *server_name) {

}

void respond_to_not_supported_request(int file_descriptor, char *server_name) {
    write_header(file_descriptor, 501, NULL, server_name);
}

int respond_to_request(char *root, int file_descriptor, char *server_name) {
    int response_size = 0;
    char mesg[READ_BUFFER_SIZE];
    // A request line has three parts, separated by spaces: a method name,
    // the local path of the requested resource, and the version of HTTP being used
    char *method_name;
    char *requested_resource_path;
    char *http_version;
    int recv_responde;

    memset((void*)mesg, (int)'\0', READ_BUFFER_SIZE);
    recv_responde = recv(file_descriptor, mesg, READ_BUFFER_SIZE, 0);
    if (recv_responde < 0) {
        fprintf(stderr, ("Error al recibir el mensaje\n"));
        close_connection(file_descriptor);
        return response_size;
    }
    if (recv_responde == 0) {
        fprintf(stderr, "El cliente se desconectó\n");
        close_connection(file_descriptor);
        return response_size;
    }
    printf("Mensaje:\n\n%sFin del mensaje\n", mesg);
    method_name = strtok(mesg, " \t\n");
    requested_resource_path = strtok(NULL, " \t");
    http_version = strtok(NULL, " \t\n");

    if (strncmp(http_version, "HTTP/1.1", 8) != 0) {
        write_header(file_descriptor, 400, NULL, server_name);
        close_connection(file_descriptor);
        return response_size;
    }
    
    if (strncmp(method_name, "GET\0", 4) == 0) {
        response_size = respond_to_get_request(root, file_descriptor, requested_resource_path, server_name);
    } else if (strncmp(method_name, "PUT\0", 4) == 0) {
        respond_to_put_request(root, file_descriptor, requested_resource_path, server_name);
    } else if (strncmp(method_name, "DELETE\0", 4) == 0) {
        respond_to_delete_request(root, file_descriptor, requested_resource_path, server_name);
    } else {
        respond_to_not_supported_request(file_descriptor, server_name);
    }

    close_connection(file_descriptor);
    printf("Process with PID %d, served %d bytes\n", getpid(), response_size);
    return response_size;
}

