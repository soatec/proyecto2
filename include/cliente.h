#ifndef PROYECTO2_CLIENTE_H
#define PROYECTO2_CLIENTE_H

#include <pthread.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "utils.h"

/**
 * Información del cliente
 */
typedef struct cliente {
  /** Dirección IP del servidor */
  char *ip;

  /** Puerto del servidor */
  uint16_t puerto;

  /** File descriptor del servidor */
  int fd;

  /** Nombre del archivo */
  char *archivo;

  /** Cantidad de threads */
  unsigned int threads;

  /** Cantidad de ciclos */
  unsigned int ciclos;

  /** Cantidad de archivos recibidos */
  unsigned int archivos_recibidos;

  /** Cantidad de errores reportados */
  unsigned int errores;

  /** Cantidad de bytes recibidos */
  unsigned int bytes_recibidos;

  /** Mutex para utilizar el socket */
  pthread_mutex_t mutex_socket;

  /** Mutex para modificar la cantidad de errores */
  pthread_mutex_t mutex_errores;

  /** Mutex para modificar la cantidad de archivos y bytes recibidos */
  pthread_mutex_t mutex_archivos;

} cliente_t;

/**
 * Iniciar cliente
 *
 * @param [inout] cliente  es la información del cliente
 *
 * @returns 0 si la operación es exitosa, sino el código de error
 *
 */
int client_init(cliente_t *cliente);

/**
 * Correr cliente
 *
 * @param [inout] cliente  es la información del cliente
 *
 * @returns 0 si la operación es exitosa, sino el código de error
 *
 */
int client_run(cliente_t *cliente);

#endif //PROYECTO2_CLIENTE_H
