#ifndef PROYECTO2_PRETHREADED_H
#define PROYECTO2_PRETHREADED_H

#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include "utils.h"

/**
 * Métodos HTTP
 */
typedef enum http_method {

  HTTP_METHOD_GET,

  HTTP_METHOD_POST,

  HTTP_METHOD_DELETE,

  HTTP_METHOD_NOT_SUPPORTED,

} http_request_t;

/**
 * Información del request HTTP
 */
typedef struct http_request {

  /** Método HTTP */
  http_method_t metodo;

  /** URI */
  char *uri;

  /** Versión HTTP */
  char *version;

} http_request_t;

/**
 * Información del response HTTP
 */
typedef struct http_response {

  /** Versión HTTP */
  char *version;

  /** Cóndigo de status */
  uint8_t status_code;

  /** Tipo del recurso */
  char *content_type;

  /** Tamaño del recurso */
  uint32_t content_length;

} http_response_t;

/**
 * Información de control de threads
 */
typedef struct thread_control {
  /** Variable para iniciar/detener el thread */
  bool run;

  /** Connection file descriptor */
  int connection_fd;

  /** Mutex para modificar la señal */
  pthread_mutex_t mutex;

  /** Señal que despierta/duerme el thread */
  pthread_cond_t signal;

} thread_control_t;

/**
 * Información del servidor prethreaded
 */
typedef struct servidor_prethreaded {

  /** File descriptor del servidor */
  int fd;

  /** Dirección IPv4 del servidor */
  struct sockaddr_in address;

  /** Cantidad de threads */
  int thread_quantity;

  /** Threads para atender solicitudes */
  pthread_t *thread_pool;

  /** Atributos para controlar los threads */
  thread_control_t *thread_control;

} servidor_prethreaded_t;

/**
 * Iniciar servidor prethreaded
 *
 * @param [in]  puerto           es el número de puerto
 * @param [in]  cantidad_threads es la cantidad de threads para atender
 *                               solicitudes
 * @param [out] servidor         es la información del servidor
 *
 * @returns 0 si la operación es exitosa, sino el código de error
 *
 */
int prethreaded_server_init(uint16_t puerto,
                            uint32_t cantidad_threads,
                            servidor_prethreaded_t *servidor);

/**
 * Correr servidor prethreaded
 *
 * @param [in] servidor  es la información del servidor
 *
 * @returns 0 si la operación es exitosa, sino el código de error
 *
 */
int prethreaded_server_run(servidor_prethreaded_t servidor);

/**
 * Finalizar servidor prethreaded
 *
 * @param [in] servidor  es la información del servidor
 *
 * @returns 0 si la operación es exitosa, sino el código de error.
 *
 */
int prethreaded_server_uninit(servidor_prethreaded_t servidor);

#endif //PROYECTO2_PRETHREADED_H
