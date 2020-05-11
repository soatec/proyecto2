#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <linux/limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>
#include "utils.h"

/*
 * Tamaño de la cola de espera del servidor
 */
#define CONNECTIONS_QUEUE_LEN 1000
#define WRITE_BUFFER_SIZE 1024
#define READ_BUFFER_SIZE 212992
#define MEDIA_TYPE_MAX_SIZE 25
#define HEAD_METHOD_NAME "HEAD\0"
#define GET_METHOD_NAME "GET\0"
#define PUT_METHOD_NAME "PUT\0"
#define DELETE_METHOD_NAME "DELETE\0"
#define ROOT "/\0"
#define CONTENT_DELIM "\r\n\r\n"
#define CONTENT_LENGTH_TAG "Content-Length"

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

// Private functions:

void close_connection(int file_descriptor) {
    // SHUT_WR = No more transmissions
    int status;
    status = shutdown(file_descriptor, SHUT_RDWR);
    if (status < 0) {
        fprintf(stderr, "Error en la función shutdown. (Errno %d: %s)\n",
                errno, strerror(errno));
    }
    status = close(file_descriptor);
    if (status < 0) {
        fprintf(stderr, "Error en la función close. (Errno %d: %s)\n",
                errno, strerror(errno));
    }
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
        case 201:
            file_size = get_file_size(file_name);
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 201 Created\r\n"
                     "Server: %s\r\n"
                     "Content-Length: 0\r\n\r\n", server_name);
            break;

        case 204:
            file_size = get_file_size(file_name);
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 204 Accepted\r\n"
                     "Server: %s\r\n"
                     "Content-Length: 0\r\n\r\n", server_name);
            break;

        case 400:
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 400 Bad Request\r\n"
                     "Server: %s\r\n"
                     "Content-Length: 0\r\n\r\n", server_name);
            break;

        case 413:
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 413 Payload Too Large\r\n"
                     "Server: %s\r\n"
                     "Content-Length: 0\r\n", server_name);
            break;
        case 404:
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 404 Not Found\r\n"
                     "Server: %s\r\n"
                     "Content-Length: 0\r\n", server_name);
            break;
        case 500:
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 500 Internal Server Error\r\n"
                     "Server: %s\r\n"
                     "Content-Length: 0\r\n\r\n", server_name);
            break;
        case 503:
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 503 Service Unavailable\r\n"
                     "Server: %s\r\n"
                     "Content-Length: 0\r\n\r\n", server_name);
            break;
        default:
        case 501:
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 501 Not Implemented\r\n"
                     "Server: %s\r\n"
                     "Content-Length: 0\r\n\r\n", server_name);
    }
    write(file_descriptor, response_header, strlen(response_header));
    printf("Header enviado:\n\n%sFin del header\n", response_header);
    return (int)strlen(response_header) + file_size;
}

int respond_to_get_and_head_request(char *root, int file_descriptor,
        char *requested_resource_path, char *server_name, bool send_body) {
    int response_size;
    char path[PATH_MAX];
    int requested_file_descriptor;
    int bytes_read;
    char data_to_send[WRITE_BUFFER_SIZE];

    if (strncmp(requested_resource_path, ROOT, strlen(ROOT) + 1) == 0) {
        requested_resource_path = "/index.html";
    }
    strcpy(path, root);
    strcpy(&path[strlen(root)], requested_resource_path);
    printf("Archivo: %s\n", path);
    requested_file_descriptor = open(path, O_RDONLY);
    if (requested_file_descriptor != -1) {
        printf("Archivo encontrado\n");
        response_size = write_header(file_descriptor, 200, path, server_name);
        if (send_body) {
            while ((bytes_read = read(requested_file_descriptor, data_to_send, WRITE_BUFFER_SIZE)) > 0) {
                response_size += write(file_descriptor, data_to_send, bytes_read);
            }
        }
    } else {
        printf("Archivo no encontrado\n");
        response_size = write_header(file_descriptor, 404, NULL, server_name);
    }
    close(requested_file_descriptor);
    return response_size;
}

int respond_to_get_request(char *root, int file_descriptor,
                            char *requested_resource_path, char *server_name) {
    return respond_to_get_and_head_request(root, file_descriptor,
                                           requested_resource_path, server_name, true);
}

int respond_to_head_request(char *root, int file_descriptor,
                           char *requested_resource_path, char *server_name) {
    return respond_to_get_and_head_request(root, file_descriptor,
                                           requested_resource_path, server_name, false);
}

int respond_to_put_request(char* content, char *root, int file_descriptor,
                           char *requested_resource_path, char *server_name) {
    int response_size = 0;
    char path[PATH_MAX];
    FILE *file_pointer;

    strcpy(path, root);
    strcpy(&path[strlen(root)], requested_resource_path);
    file_pointer = fopen(path, "w");
    if(file_pointer == NULL) {
        response_size = write_header(file_descriptor, 500, NULL, server_name);
        fprintf(stderr, "Error ejecutando la función fopen. (Errno. %d: %s %s)\n",
                errno, strerror(errno), path);
        return response_size;
    }
    fputs(content, file_pointer);
    fclose(file_pointer);
    response_size = write_header(file_descriptor, 201, NULL, server_name);
    return response_size;
}

int respond_to_delete_request(char *root, int file_descriptor,
        char *resource_path_to_delete, char *server_name) {
    int response_size;
    char path[PATH_MAX];
    int access_response;
    int remove_response;

    if (strncmp(resource_path_to_delete, ROOT, strlen(ROOT) + 1) == 0) {
        write_header(file_descriptor, 400, NULL, server_name);
    }

    strcpy(path, root);
    strcpy(&path[strlen(root)], resource_path_to_delete);
    access_response = access(path, F_OK);
    if (access_response != -1) {
        printf("Archivo encontrado\n");
        remove_response = remove(path);
        if (remove_response == 0) {
            response_size = write_header(file_descriptor, 204, NULL, server_name);
            printf("Archivo borrado exitosamente\n");
        } else {
            response_size = write_header(file_descriptor, 500, NULL, server_name);
            fprintf(stderr, "Error ejecutando la función remove. (Errno. %d: %s)\n",
                    errno, strerror(errno));
        }
    } else {
        printf("Archivo no encontrado\n");
        response_size = write_header(file_descriptor, 404, NULL, server_name);
    }
    return response_size;
}

int respond_to_not_supported_request(int file_descriptor, char *server_name) {
    return write_header(file_descriptor, 501, NULL, server_name);
}

// Public functions:

int respond_to_request(char *root, int file_descriptor, char *server_name) {
    int response_size = 0;
    char message[READ_BUFFER_SIZE];
    // A request line has three parts, separated by spaces: a method name,
    // the local path of the requested resource, and the version of HTTP being used
    char *method_name;
    char *resource_path;
    char *http_version;
    char *content;
    char *content_length_str;
    int content_length_int;
    int recv_responde;

    memset((void*)message, (int)'\0', READ_BUFFER_SIZE);
    recv_responde = recv(file_descriptor, message, READ_BUFFER_SIZE, 0);
    if (recv_responde < 0) {
        fprintf(stderr, ("Error al recibir el mensaje\n"));
        response_size = write_header(file_descriptor, 500, NULL, server_name);
        close_connection(file_descriptor);
        return response_size;
    }
    if (recv_responde == 0) {
        fprintf(stderr, "El cliente se desconectó\n");
        close_connection(file_descriptor);
        return response_size;
    }
    printf("\n\nMensaje:\n%s\nFin del mensaje\n\n", message);

    content = strstr(message, CONTENT_DELIM) + strlen(CONTENT_DELIM);
    content_length_str = strstr(message, CONTENT_LENGTH_TAG);
    if (content_length_str != NULL) {
        content_length_str = strtok(content_length_str  + strlen(CONTENT_LENGTH_TAG) + 2, "\r\n");
        content_length_int = atoi(content_length_str);
        if (content_length_int != strlen(content)) {
            write_header(file_descriptor, 413, NULL, server_name);
            close_connection(file_descriptor);
            return response_size;
        }
    }
    method_name = strtok(message, " \t\n");
    resource_path = strtok(NULL, " \t");
    http_version = strtok(NULL, " \t\n");

    if (strncmp(http_version, "HTTP/1.1", 8) != 0) {
        write_header(file_descriptor, 400, NULL, server_name);
        close_connection(file_descriptor);
        return response_size;
    }

    if (strncmp(method_name, GET_METHOD_NAME, strlen(HEAD_METHOD_NAME)) == 0) {
        response_size = respond_to_get_request(root, file_descriptor, resource_path, server_name);
    } else if (strncmp(method_name, HEAD_METHOD_NAME, strlen(HEAD_METHOD_NAME)) == 0) {
        response_size = respond_to_head_request(root, file_descriptor, resource_path, server_name);
    } else if (strncmp(method_name, PUT_METHOD_NAME, strlen(HEAD_METHOD_NAME)) == 0) {
        response_size = respond_to_put_request(content, root, file_descriptor, resource_path, server_name);
    } else if (strncmp(method_name, DELETE_METHOD_NAME, strlen(HEAD_METHOD_NAME)) == 0) {
        response_size = respond_to_delete_request(root, file_descriptor, resource_path, server_name);
    } else {
        response_size = respond_to_not_supported_request(file_descriptor, server_name);
    }

    close_connection(file_descriptor);
    printf("Process with PID %d, served %d bytes\n", getpid(), response_size);
    return response_size;
}

int respond_internal_server_error(int file_descriptor, char *server_name) {
    return write_header(file_descriptor, 500, NULL, server_name);
}

int respond_service_unavailable(int file_descriptor, char *server_name) {
    return write_header(file_descriptor, 503, NULL, server_name);
}

int tcp_connection_init(uint16_t puerto, char *direccion_ip,
                        bool servidor) {
  int status;
  int  fd;
  struct sockaddr_in address;
  int reuse_addr = 1;

  // Crear socket TCP (IPv4)
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    fprintf(stderr, "Error %d al crear socket TCP\n", errno);
    return -1;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(int)) < 0) {
    fprintf(stderr, "Error %d al activar el flag reuse_addr del socket TCP\n",
    errno);
    return -1;
  }

  // Inicializar dirección IP
  address.sin_family = AF_INET;         // IPv4
  address.sin_port = htons(puerto);     // Número de puerto en network byte order
  if (direccion_ip == NULL) {
    address.sin_addr.s_addr = INADDR_ANY; // Cualquier dirección de enlace
  } else {
    status = inet_aton(direccion_ip, &address.sin_addr);
    if (status == 0) {
      fprintf(stderr, "Error al convertir dirección IP\n");
      return -1;
    }
  }

  /* Diferencia entre el tamaño de sockaddr y sockaddr_in, se debe limpiar
     para evitar bugs */
  memset(&address.sin_zero, 0, sizeof(address.sin_zero));

  if (servidor) {
    // Asignar dirección IP al socket
    status = bind(fd, (struct sockaddr *)&address, sizeof(address));
    if (status) {
      fprintf(stderr, "Error %d al asignar una dirección"
              " IPv4 al socket TCP\n", errno);
      return -1;
    }

    // Esperar conexiones de clientes
    status = listen(fd, CONNECTIONS_QUEUE_LEN);
    if (status) {
      fprintf(stderr, "Error %d al esperar conexiones en"
                      " el socket TCP\n", errno);
      return -1;
    }
  } else {
    // Conectar al sevidor
    status = connect(fd, (struct sockaddr *)&address, sizeof(address));
    if (status) {
      fprintf(stderr, "Error %d al tratar de contactar al servidor"
              " IPv4 al socket TCP\n", errno);
      return -1;
    }
  }

  return fd;
}

int tcp_connection_uninit(int fd) {
int status;

  // Finalizar el flujo de datos
  status = shutdown(fd, SHUT_RDWR);
  if (status) {
    fprintf(stderr, "Error %d al cerrar el file"
                    " descriptor del servidor\n", errno);
    return -1;
  }

  // Cerrar el file descriptor del servidor
  status = close(fd);
  if (status) {
    fprintf(stderr, "Error %d al cerrar el file"
                    " descriptor del servidor\n", errno);
    return -1;
  }

  return 0;
}

int send_get_request(int file_descriptor, char *file_location, int times) {
    int response_size = 0;
    int bytes_read;
    char mesg[READ_BUFFER_SIZE];
    char received_data[WRITE_BUFFER_SIZE];
    int status;
    char *http_header;
    char *version;
    char *status_code;
    char *status_phrase;
    char *file_name;

    memset((void*)mesg, (int)'\0', READ_BUFFER_SIZE);

    /* A request line has three parts, separated by spaces: a method name,
       the local path of the requested resource, and the version of HTTP
       being used */
    sprintf(mesg, "GET %s HTTP/1.1\n", file_location);

    printf("HTTP Request:\n%sFin del request\n", mesg);

    status = write(file_descriptor, mesg, strlen(mesg));
    if (status < 0) {
        fprintf(stderr, ("Error al enviar HTTP GET request\n"));
        close_connection(file_descriptor);
        return response_size;
    }
    if (status == 0) {
        fprintf(stderr, "El servidor se desconectó\n");
        close_connection(file_descriptor);
        return response_size;
    }

    status = read(file_descriptor, mesg, READ_BUFFER_SIZE);
    if (status < 0) {
        fprintf(stderr, ("Error al recibir HTTP response\n"));
        close_connection(file_descriptor);
        return response_size;
    }
    if (status == 0) {
        fprintf(stderr, "El servidor se desconectó\n");
        close_connection(file_descriptor);
        return response_size;
    }

    /* An HTTP response has 3 parts, separated with spaces: HTTP version,
      response status code, reason phrase describing the status code */
    version       = strtok(mesg, " \t\n");
    status_code   = strtok(NULL, " \t");
    status_phrase = strtok(NULL, " \t\n");

    if (strncmp(version, "HTTP/1.1", 8) != 0) {
        fprintf(stderr, "Unsupported version:%s\n", version);
        close_connection(file_descriptor);
        return response_size;
    }

    if (strncmp(status_code, "200", 3) != 0) {
        fprintf(stderr, "HTTP request failed with code:%s(%s)\n", status_code,
                status_phrase);
        close_connection(file_descriptor);
        return response_size;
    }

    printf("HTTP response:\n%s %s %s\n", version, status_code, status_phrase);
    while((http_header = strtok(NULL, "\t\n")) != NULL) {
      printf("%s\n", http_header);
    }
    printf("Fin del response\n\n");

    file_name = strrchr(file_location, '/');
    if (file_name != NULL) {
      file_name+=1;
    } else {
      file_name = file_location;
    }

    printf("Requested file name: %s\n", file_name);

    while ((bytes_read=read(file_descriptor, received_data, WRITE_BUFFER_SIZE)) > 0) {
        response_size += bytes_read;
    }

    close_connection(file_descriptor);
    printf("Process with PID %d, received %d bytes\n", getpid(), response_size);
    return response_size;
}
