#ifndef PROYECTO2_PRETHREADED_H
#define PROYECTO2_PRETHREADED_H

#include <pthread.h>
#include <stdbool.h>
#include "utils.h"


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

  /** Root path del servidor */
  char *root_path;

} thread_control_t;

/**
 * Información del servidor prethreaded
 */
typedef struct servidor_prethreaded {

  /** File descriptor del servidor */
  int fd;

  /** Cantidad de threads */
  int thread_quantity;

  /** Threads para atender solicitudes */
  pthread_t *thread_pool;

  /** Atributos para controlar los threads */
  thread_control_t *thread_control;

  /** Flag to run/stop server */
  bool run;

} servidor_prethreaded_t;

/**
 * Iniciar servidor prethreaded
 *
 * @param [in]  puerto           es el número de puerto
 * @param [in]  cantidad_threads es la cantidad de threads para atender
 *                               solicitudes
 * @param [in]  root_path        es el root path del servidor
 * @param [out] servidor         es la información del servidor
 *
 * @returns 0 si la operación es exitosa, sino el código de error
 *
 */
int prethreaded_server_init(uint16_t                puerto,
                            uint32_t                cantidad_threads,
                            char                   *root_path,
                            servidor_prethreaded_t *servidor);

/**
 * Correr servidor prethreaded
 *
 * @param [in] servidor  es la información del servidor
 *
 * @returns 0 si la operación es exitosa, sino el código de error
 *
 */
int prethreaded_server_run(servidor_prethreaded_t *servidor);

/**
 * Finalizar servidor prethreaded
 *
 * @param [in] servidor  es la información del servidor
 *
 * @returns 0 si la operación es exitosa, sino el código de error.
 *
 */
int prethreaded_server_uninit(servidor_prethreaded_t *servidor);

#endif //PROYECTO2_PRETHREADED_H
