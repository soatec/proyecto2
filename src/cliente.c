#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include "cliente.h"
#include "utils.h"


/*
 * Tamaño del buffer en bytes
 */
#define BUFFER_LENGTH 8192


/*
 * Iniciar cliente
 */
int client_init(cliente_t *cliente) {

  // Inicializar conexión TCP
  cliente->fd = tcp_connection_init(cliente->puerto, cliente->ip, false);
  if (cliente->fd < 0) {
    fprintf(stderr, "[Cliente] Error al iniciar conexión TPC\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/*
 * Correr cliente
 */
int client_run(cliente_t *cliente) {
  int status;
  int bytes_recibidos;

  status = pthread_mutex_lock(&cliente->mutex_socket);
  if (status) {
    fprintf(stderr, "[Cliente] Error %d al obtener el lock del socket\n",
            status);
    return EXIT_FAILURE;
  }

  bytes_recibidos = send_get_request(cliente->fd, cliente->archivo,
                                     cliente->ciclos);

  status = pthread_mutex_unlock(&cliente->mutex_socket);
  if (status) {
    fprintf(stderr, "[Cliente] Error %d al liberar el lock de socket\n",
            status);
   return EXIT_FAILURE;
  }

  if (bytes_recibidos > 0) {
    status = pthread_mutex_lock(&cliente->mutex_archivos);
    if (status) {
      fprintf(stderr, "[Cliente] Error %d al obtener el lock de archivos\n",
              status);
      return EXIT_FAILURE;
    }
    cliente->archivos_recibidos++;
    cliente->bytes_recibidos += bytes_recibidos;
    status = pthread_mutex_unlock(&cliente->mutex_archivos);
    if (status) {
      fprintf(stderr, "[Cliente] Error %d al liberar el lock de archivos\n",
              status);
      return EXIT_FAILURE;
    }
  } else {
    status = pthread_mutex_lock(&cliente->mutex_errores);
    if (status) {
      fprintf(stderr, "[Cliente] Error %d al obtener el lock de errores\n",
              status);
      return EXIT_FAILURE;
    }
     if (bytes_recibidos < 0) {
        cliente->errores++;
      } else {
        cliente->rw_cero++;
      }
    status = pthread_mutex_unlock(&cliente->mutex_errores);
    if (status) {
      fprintf(stderr, "[Cliente] Error %d al liberar el lock de errores\n",
              status);
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
