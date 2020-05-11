#ifndef PROYECTO2_SECUENCIAL_H
#define PROYECTO2_SECUENCIAL_H

#include "utils.h"
extern struct cache *cache_create(int max_size, int hashsize);
int get_listener_socket(char *address, char *port);
void handle_http_request(int fd);
void *get_in_addr(struct sockaddr *sa);
#endif //PROYECTO2_SECUENCIAL_H
