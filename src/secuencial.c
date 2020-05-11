#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "secuencial.h"
#include "utils.h"

#define BACKLOG 10	 // how many pending connections queue will hold

struct file_data {
    int size;
    void *data;
};

/**
 * Loads a file into memory and returns a pointer to the data.
 * 
 * Buffer is not NUL-terminated.
 */
struct file_data *file_load(char *filename)
{
    char *buffer, *p;
    struct stat buf;
    int bytes_read, bytes_remaining, total_bytes = 0;

    // Get the file size
    if (stat(filename, &buf) == -1) {
        return NULL;
    }

    // Make sure it's a regular file
    if (!(buf.st_mode & S_IFREG)) {
        return NULL;
    }

    // Open the file for reading
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL) {
        return NULL;
    }

    // Allocate that many bytes
    bytes_remaining = buf.st_size;
    p = buffer = malloc(bytes_remaining);

    if (buffer == NULL) {
        return NULL;
    }

    // Read in the entire file
    while (bytes_read = fread(p, 1, bytes_remaining, fp), bytes_read != 0 && bytes_remaining > 0) {
        if (bytes_read == -1) {
            free(buffer);
            return NULL;
        }

        bytes_remaining -= bytes_read;
        p += bytes_read;
        total_bytes += bytes_read;
    }

    // Allocate the file data struct
    struct file_data *filedata = malloc(sizeof *filedata);

    if (filedata == NULL) {
        free(buffer);
        return NULL;
    }

    filedata->data = buffer;
    filedata->size = total_bytes;

    return filedata;
}

/**
 * Frees memory allocated by file_load().
 */
void file_free(struct file_data *filedata)
{
    free(filedata->data);
    free(filedata);
}



/**
 * This gets an Internet address, either IPv4 or IPv6
 *
 * Helper function to make printing easier.
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int send_response(int fd, char *header, char *content_type, char *body, int content_length)
{
	const int max_response_size = 262144;
    char response[max_response_size];
	int response_length = strlen(body);
	//int response_body_length = 
	time_t t = time(NULL);
    struct tm *local_time = localtime(&t);
    char *timestamp = asctime(local_time);
    
	response_length = sprintf(response, "%s\nDate %s Connection: close\nContent-Type: %s\nContent-Length: %d\n\n", header, timestamp, content_type, content_length);
    memcpy(response + response_length, body, content_length);
    response_length += content_length;
	
	int rv = send(fd, response, response_length, 0);

	if (rv < 0)
	{
		perror("send");
	}

	return rv;
}

void set_d404(int fd, char *path)
{
  char response_body[1024];
  int string_length;
  sprintf(response_body, "404: %s not found", path);
  string_length = snprintf(response_body, sizeof(response_body), "404: %s not found", path);
  send_response(fd, "HTTP/1.1 404 NOT FOUND", "text/html", response_body,string_length);
}



void get_file(int fd,char *request_path)
{
 
	char filepath[4096];
	struct file_data *filedata; 
	char *mime_type;
	//char response_body[1024];

    if (strcmp(request_path, "/") == 0) {
        snprintf(filepath, sizeof filepath, "%s/index.html", "./serverroot");
    } else {
        snprintf(filepath, sizeof filepath, "%s%s", "./serverroot", request_path);
    }
    
    filedata = file_load(filepath);
    
    
    if (filedata == NULL) {
        set_d404(fd,request_path);
        return;
    } else {
        mime_type = strrchr(request_path, '.');
        //cache_put(cache, filepath, mime_type, filedata->data, filedata->size);
        send_response(fd, "HTTP/1.1 200 OK", mime_type, filedata->data, filedata->size);
    }

   
   if (filedata == NULL) {
        set_d404(fd,request_path);
        return;
    } else {
        mime_type = strrchr(request_path, '.');
        //cache_put(cache, filepath, mime_type, filedata->data, filedata->size);
        send_response(fd, "HTTP/1.1 200 OK", mime_type, filedata->data, filedata->size);
    }

    file_free(filedata);
/*

  FILE *fp;

  fp = fopen("root.txt", "r");
  fread(response_body, 8, sizeof(response_body), fp);
  fclose(fp);

  send_response(fd, "HTTP/1.1 200 OK", "text/html", response_body);
}

*/
}

void post_save(int fd, char *body)
{
	char response_body[1024];
	int string_length;
	
	FILE *fp;
	fp = fopen("root.txt", "w+");
	fwrite(body, 8, sizeof(body), fp);
    fclose(fp);
    string_length = sprintf(response_body, "{\"status\":\"ok\"}");
    send_response(fd, "HTTP/1.1 201 OK", "application/json", response_body,string_length);
}



void get_d200ok(int fd)
{
	char *response_body = malloc(sizeof(int));
	int num;
	int string_length;
	do
	{
		num = rand() % 100;
	}while (num - 20 > 0 || num - 1 < 0);

	string_length = sprintf(response_body, "%d", num);
	send_response(fd, "HTTP/1.1 200 OK", "text/plain", response_body,string_length);
}

/**
 * Return the main listening socket
 *
 * Returns -1 or error
 */
int get_listener_socket(char *address, char *port)
{
    int sockfd; 
    struct addrinfo hints, *servinfo, *p;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }
	
	
    for(p = servinfo; p != NULL; p = p->ai_next) {

		
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
			printf(" sockfd  error\n");	
            continue;
        }
		
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
            sizeof(int)) == -1) {
			printf(" setsockopt error\n");
            close(sockfd);
            freeaddrinfo(servinfo); 
            return -2;
        }


        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            printf(" setsockopt error\n");
            close(sockfd);
            continue;
        }
		
	

        break;
    }

    freeaddrinfo(servinfo); 
    printf(" freeaddrinfo\n");

    if (p == NULL)  {
        fprintf(stderr, "webserver: failed to find local address\n");
        return -3;
    }

	
 
    if (listen(sockfd, BACKLOG) == -1) {
        printf(" perror listen\n");
        close(sockfd);
        return -4;
    }
	
    return sockfd;
}


char *find_header(char *header)
{
	int lines = 9;
	char *newlines[9] = {
      "\n\n",
      "\n\r",
      "\r\n",
      "\r\r",
      /* */
      "\r\r\n",
      "\n\r\n",
      "\r\n\n",
      "\r\n\r",
      /* */
      "\r\n\r\n"};

	  for (int i = 0; i < lines; i++)
	  {
		char *newline = newlines[i];
		char *addr = strstr(header, newline);

		if (addr)
		{
		  return addr + strlen(newline) + 1; /* account for \0 */
		}
	  }
	  
  return header;
}
/**
 * Handle HTTP request and send response
 */
void handle_http_request(int fd)
{
    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];
    char *head;
    char request_type[8];       // GET or POST
	char request_path[1024];    // /info etc.
	char request_protocol[128]; // HTTP/1.1
	
	
    // Read request
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0) {
        perror("recv");
        return;
    }


	request[bytes_recvd] = '\0';
	
	 sscanf(request, "%s %s %s", request_type, request_path, request_protocol);
	 
	 head = find_header(request);

	if (strcmp(request_type, "POST") == 0)
	{
		if (strcmp(request_path, "/save") == 0)
		{
		  post_save(fd, head);
		  return;
		}
	}
	
	if (strcmp(request_type, "GET") == 0)
	{
		if (strcmp(request_path, "/") == 0)
		{
		  get_file(fd,request_path);
		  return;
		}
		
		if (strcmp(request_path, "/d20") == 0)
		{
		  get_d200ok(fd);
		  return;
		}
		
	}
	
	set_d404(fd, request_path);
   
    return;
   
}



