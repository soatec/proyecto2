#ifndef PROYECTO2_FORKED_H
#define PROYECTO2_FORKED_H

#include "utils.h"

/**
 *
 * @param port_int
 * @return
 */
int init_forked_server(int port_int);

/**
 *
 * @param socket_file_descriptor
 * @param root
 * @return
 */
int execute_forked_server(int socket_file_descriptor, char *root);

#endif //PROYECTO2_FORKED_H
