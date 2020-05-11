#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "prethreaded.h"
#include "utils.h"

/*
 * Tamaño del buffer en bytes
 */
#define BUFFER_LENGTH 8192

#define SERVER_NAME "Prethreaded server"


/*
 * Rutina para atender requests
 */
void *address_request(void *args) {
  int status;
  thread_control_t *thread_control = (thread_control_t *)args;
  char *server_name = SERVER_NAME;

  while(thread_control->run) {
    status = pthread_mutex_lock(&thread_control->mutex);
    if(status){
      fprintf(stderr, "[Servidor Prethreaded] Error %d al bloquear el mutex"
                       " de un thread\n", status);
    }

    status = pthread_cond_wait(&thread_control->signal, &thread_control->mutex);
    if(status) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al poner en espera un"
                      " thread\n", status);
    }

    if (!thread_control->run) break;

    respond_to_request(thread_control->root_path, thread_control->connection_fd,
                       server_name);

    status = pthread_mutex_unlock(&thread_control->mutex);
    if(status){
      fprintf(stderr, "[Servidor Prethreaded] Error %d al desbloquear el mutex"
                       " de un thread\n", status);
    }

  }

  // Desbloquear el mutex antes de salir
  status = pthread_mutex_unlock(&thread_control->mutex);
  if(status){
    fprintf(stderr, "[Servidor Prethreaded] Error %d al desbloquear el mutex"
                     " de un thread\n", status);
  }

  pthread_exit(NULL);
}

/*
 * Iniciar servidor prethreaded
 */
int prethreaded_server_init(uint16_t                puerto,
                            uint32_t                cantidad_threads,
                            char                   *root_path,
                            servidor_prethreaded_t *servidor) {
  int status;

  // Inicializar conexión TCP
  servidor->fd = tcp_connection_init(puerto, NULL, true);
  if (servidor->fd == -1) {
    fprintf(stderr, "[Servidor Prethreaded] Error al iniciar conexión TPC\n");
    return EIO;
  }

  // Inicializar el pool de threads para atender solicitudes
  servidor->thread_quantity = cantidad_threads;
  servidor->thread_pool = (pthread_t *)malloc(servidor->thread_quantity *
                                              sizeof(pthread_t));
  if (servidor->thread_pool == NULL) {
    status = ENOMEM;
    fprintf(stderr, "[Servidor Prethreaded] Error %d al hacer alloc de thread"
                     " pool\n", status);
    return status;
  }

  servidor->thread_control =
    (thread_control_t *)malloc(servidor->thread_quantity *
                               sizeof(thread_control_t));
  if (servidor->thread_control == NULL) {
    status = ENOMEM;
    fprintf(stderr, "[Servidor Prethreaded] Error %d al hacer alloc de thread"
                    " control\n", status);
    goto error_tp;
  }

  for(int i = 0; i < cantidad_threads; i++) {

    // Iniciar mutex y cond utilizado para enviar señales a los threads
    status = pthread_mutex_init(&servidor->thread_control[i].mutex, NULL);
    if(status){
      fprintf(stderr, "[Servidor Prethreaded] Error %d al iniciar el mutex del"
                       " thread número %d\n", status, i);
      goto error_tc;
    }

    status = pthread_cond_init(&servidor->thread_control[i].signal, NULL);
    if(status){
      fprintf(stderr, "[Servidor Prethreaded] Error %d al iniciar el cond del"
                       " thread número %d\n", status, i);
      goto error_tc;
    }

    // Indicar al thread que debe iniciar la ejecución
    servidor->thread_control[i].run = true;

    // Indicar el root path del servidor
    servidor->thread_control[i].root_path = root_path;

    // Crear thread con la rutina de atención de requests
    status = pthread_create(&servidor->thread_pool[i], NULL, address_request,
                            &servidor->thread_control[i]);
    if(status){
      fprintf(stderr, "[Servidor Prethreaded] Error %d al crear thread número"
                       " %d\n", status, i);
      goto error_tc;
    }

  }

  servidor->run = true;

  return EXIT_SUCCESS;

error_tc:
  free(servidor->thread_control);
error_tp:
  free(servidor->thread_pool);

  return status;
}

/*
 * Correr servidor prethreaded
 */
int prethreaded_server_run(servidor_prethreaded_t *servidor) {
  int request_fd;
  int status;
  struct sockaddr_in client_address;
  socklen_t address_len = sizeof(client_address);

  // TODO: Hacer loop para aceptar conexiones

  while(servidor->run) {
    request_fd = accept(servidor->fd, (struct sockaddr *)&client_address,
                        &address_len);
    if (request_fd < 0) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al aceptar una "
              "conexión\n", errno);
      return status;
    }

    printf("[Servidor Prethreaded] Atendiendo al cliente %s\n",
           inet_ntoa(client_address.sin_addr));

    for(int i = 0; i < servidor->thread_quantity; i++) {

      // Si obtenemos el lock de un thread, significa que está desocupado
      status = pthread_mutex_trylock(&servidor->thread_control[i].mutex);
      if (status) {
        if (status == EBUSY) continue;
        fprintf(stderr, "[Servidor Prethreaded] Error %d al obtener el lock del "
                        "thread número %d\n", status, i);
        goto error;
      }

      printf("Asignando file descriptor %d al thread %d\n", request_fd, i);

      // Asignar el file descriptor de la conexión
      servidor->thread_control[i].connection_fd = request_fd;

      // Enviar señal para despertar al thread
      status = pthread_cond_signal(&servidor->thread_control[i].signal);
      if (status) {
        fprintf(stderr, "[Servidor Prethreaded] Error %d al hacerle signal al "
                        "thread número %d\n", status, i);
        goto error;
      }

      // Liberar el lock del thread
      status = pthread_mutex_unlock(&servidor->thread_control[i].mutex);
      if (status) {
        if (status == EBUSY) continue;
        fprintf(stderr, "[Servidor Prethreaded] Error %d al liberar el lock del "
                        "thread número %d\n", status, i);
        return status;
      }
      break;
    }
  }

  return EXIT_SUCCESS;

error:
  free(servidor->thread_control);
  free(servidor->thread_pool);

  return status;
}

/*
 * Finalizar servidor prethreaded
 */
int prethreaded_server_uninit(servidor_prethreaded_t *servidor) {
  int status;

  printf("[Servidor Prethreaded] Desinicializando servidor\n");

  servidor->run = false;

  status = tcp_connection_uninit(servidor->fd);
  if (status) goto error_free;

  /* Despertar a los threads indicando que deben detenerse, destruir el cond y
     mutex de cada thread */
  for(int i=0; i<servidor->thread_quantity; i++) {

    // Si obtenemos el lock de un thread, significa que está desocupado
    status = pthread_mutex_lock(&servidor->thread_control[i].mutex);
    if (status) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al obtener el lock de "
                      "un thread de atención de solicitudes\n", status);
      goto error_free;
    }

    // Indicar al thread que debe detenerse
    servidor->thread_control[i].run = false;

    // Enviar señal para despertar al thread
    status = pthread_cond_signal(&servidor->thread_control[i].signal);
    if (status) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al hacerle signal al "
                      "thread número %d\n", status, i);
      goto error_free;
    }

    status = pthread_mutex_unlock(&servidor->thread_control[i].mutex);
    if (status) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al desbloquear el lock "
                      "del thread de número %d\n", status, i);
      goto error_free;
    }

    /* Hacer join del thread para que el proceso principal espere a que
       termine */
    status = pthread_join(servidor->thread_pool[i], NULL);
    if(status){
      fprintf(stderr, "[Servidor Prethreaded] Error %d al crear hacer join"
                      " del thread número %d\n", status, i);
      goto error_free;
    }

    status = pthread_mutex_destroy(&servidor->thread_control[i].mutex);
    if(status){
      fprintf(stderr, "[Servidor Prethreaded] Error %d al destruir el mutex"
                       " del thread número %d\n", status, i);
      goto error_free;
    }

    status = pthread_cond_destroy(&servidor->thread_control[i].signal);
    if(status) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al destruir el cond del"
                      " thread número %d\n", status, i);
      goto error_free;
    }

  }

  // Liberar thread control y thread pool
  free(servidor->thread_control);
  free(servidor->thread_pool);

  return EXIT_SUCCESS;

error_free:
  free(servidor->thread_control);
  free(servidor->thread_pool);

  return status;
}
