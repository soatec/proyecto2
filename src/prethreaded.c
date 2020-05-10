#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include "prethreaded.h"
#include "utils.h"

/*
 * Tamaño del buffer en bytes
 */
#define BUFFER_LENGTH 8192

/*
 * Tamaño de la cola de espera del servidor
 */
#define CONNECTIONS_QUEUE_LEN 1000

/*
 * Parse HTTP request
 */
http_request_t parse_request(int fd) {
  http_request_t ret;
  FILE *file_stream = fdopen(fd, "r");

  if (file_stream == NULL)
  {
      fprintf(stderr, "Error opening file descriptor in getMessage()\n");
      exit(EXIT_FAILURE);
  }
  return ret;
}

/*
 * Rutina para atender requests
 */
void *address_request(void *args) {
  int status;
  long request_size;
  char buffer[BUFFER_LENGTH];
  thread_control_t *thread_control = (thread_control_t *)args;
  char *response = "Hola del servidor\n";

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

    // Recibir solicitud
    request_size = read(thread_control->connection_fd, buffer, BUFFER_LENGTH);
    if (request_size < 0) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al leer un mensaje\n",
              errno);
    }

    // TODO: Parse request
    printf("RECEIVED MESSAGE: %s\n", buffer);

    // TODO: Make response

    // Enviar respuesta
    request_size = write(thread_control->connection_fd, response,
                         strlen(response));
    if (request_size < 0) {
     fprintf(stderr, "[Servidor Prethreaded] Error %d al enviar un mensaje\n",
             errno);
    }

    // Cerrar el file descriptor de la conexión
    status = close(thread_control->connection_fd);
    if (status) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al cerrar el file "
                      "descriptor de la conexión\n", errno);
    }

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
int prethreaded_server_init(uint16_t puerto,
                            uint32_t cantidad_threads,
                            servidor_prethreaded_t *servidor) {
  int status;
  int fd;
  struct sockaddr_in address;

  // Crear socket TCP (IPv4)
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    fprintf(stderr, "[Servidor Prethreaded] Error %d al crear socket TCP\n",
            errno);
    return errno;
  }

  // Inicializar dirección IP
  address.sin_family = AF_INET;         // IPv4
  address.sin_port = htons(puerto);     // Número de puerto en network byte order
  address.sin_addr.s_addr = INADDR_ANY; // Cualquier dirección de enlace

  /* Diferencia entre el tamaño de sockaddr y sockaddr_in, se debe limpiar
     para evitar bugs */
  memset(&address.sin_zero, 0, sizeof(address.sin_zero));

  // Asignar dirección IP al socket
  status = bind(fd, (struct sockaddr *)&address, sizeof(address));
  if (status) {
    fprintf(stderr, "[Servidor Prethreaded] Error %d al asignar una dirección"
            " IPv4 al socket TCP\n", errno);
    return errno;
  }

  // Asignar el file descriptor y la dirección del servidor
  servidor->fd      = fd;
  servidor->address = address;

  // Inicializar el pool de threads para atender solicitudes
  servidor->thread_pool = (pthread_t *)malloc(servidor->thread_quantity *
                                              sizeof(pthread_t));
  if (servidor->thread_pool == NULL) {
    status = -ENOMEM;
    fprintf(stderr, "[Servidor Prethreaded] Error %d al hacer alloc de thread"
                     " pool\n", status);
    return status;
  }

  servidor->thread_control =
    (thread_control_t *)malloc(servidor->thread_quantity *
                               sizeof(thread_control_t));
  if (servidor->thread_control == NULL) {
    status = -ENOMEM;
    fprintf(stderr, "[Servidor Prethreaded] Error %d al hacer alloc de thread"
                    " control\n", status);
    return status;
  }

  for(int i = 0; i < cantidad_threads; i++) {

    // Iniciar mutex y cond utilizado para enviar señales a los threads
    status = pthread_mutex_init(&servidor->thread_control[i].mutex, NULL);
    if(status){
      fprintf(stderr, "[Servidor Prethreaded] Error %d al iniciar el mutex del"
                       " thread número %d\n", status, i);
      return status;
    }

    status = pthread_cond_init(&servidor->thread_control[i].signal, NULL);
    if(status){
      fprintf(stderr, "[Servidor Prethreaded] Error %d al iniciar el cond del"
                       " thread número %d\n", status, i);
      return status;
    }

    // Indicar al thread que debe iniciar la ejecución
    servidor->thread_control[i].run = true;

    // Crear thread con la rutina de atención de requests
    status = pthread_create(&servidor->thread_pool[i], NULL, address_request,
                            &servidor->thread_control[i]);
    if(status){
      fprintf(stderr, "[Servidor Prethreaded] Error %d al crear thread número"
                       " %d\n", status, i);
      return status;
    }

  }

  // Asignar cantidad de threads a la información del servidor
  servidor->thread_quantity = cantidad_threads;

  // Esperar conexiones de clientes
  status = listen(fd, CONNECTIONS_QUEUE_LEN);
  if (status) {
    fprintf(stderr, "[Servidor Prethreaded] Error %d al esperar conexiones en"
                    " el socket TCP\n", errno);
    return errno;
  }

  return EXIT_SUCCESS;
}

/*
 * Correr servidor prethreaded
 */
int prethreaded_server_run(servidor_prethreaded_t servidor) {
  int request_fd;
  int status;
  socklen_t address_len = sizeof(servidor.address);

  // TODO: Hacer loop para aceptar conexiones

  request_fd = accept(servidor.fd, (struct sockaddr *)&servidor.address,
                      &address_len);
  if (request_fd < 0) {
    fprintf(stderr, "[Servidor Prethreaded] Error %d al aceptar una "
            "conexión\n", errno);
    return errno;
  }

  for(int i = 0; i < servidor.thread_quantity; i++) {

    // Si obtenemos el lock de un thread, significa que está desocupado
    status = pthread_mutex_trylock(&servidor.thread_control[i].mutex);
    if (status) {
      if (status == EBUSY) continue;
      fprintf(stderr, "[Servidor Prethreaded] Error %d al obtener el lock del "
                      "thread número %d\n", status, i);
      return status;
    }

    // Asignar el file descriptor de la conexión
    servidor.thread_control[i].connection_fd = request_fd;

    // Enviar señal para despertar al thread
    status = pthread_cond_signal(&servidor.thread_control[i].signal);
    if (status) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al hacerle signal al "
                      "thread número %d\n", status, i);
      return status;
    }

    // Liberar el lock del thread
    status = pthread_mutex_unlock(&servidor.thread_control[i].mutex);
    if (status) {
      if (status == EBUSY) continue;
      fprintf(stderr, "[Servidor Prethreaded] Error %d al liberar el lock del "
                      "thread número %d\n", status, i);
      return status;
    }
  }

  return EXIT_SUCCESS;
}

/*
 * Finalizar servidor prethreaded
 */
int prethreaded_server_uninit(servidor_prethreaded_t servidor) {
  int status;

  // Cerrar el file descriptor del servidor
  status = close(servidor.fd);
  if (status) {
    fprintf(stderr, "[Servidor Prethreaded] Error %d al cerrar el file"
                    " descriptor del servidor\n", errno);
    return errno;
  }

  /* Despertar a los threads indicando que deben detenerse, destruir el cond y
     mutex de cada thread */
  for(int i=0; i<servidor.thread_quantity; i++) {

    // Si obtenemos el lock de un thread, significa que está desocupado
    status = pthread_mutex_lock(&servidor.thread_control[i].mutex);
    if (status) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al obtener el lock de "
                      "un thread de atención de solicitudes\n", status);
      return status;
    }

    // Indicar al thread que debe detenerse
    servidor.thread_control[i].run = false;

    // Enviar señal para despertar al thread
    status = pthread_cond_signal(&servidor.thread_control[i].signal);
    if (status) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al hacerle signal al "
                      "thread número %d\n", status, i);
      return status;
    }

    status = pthread_mutex_unlock(&servidor.thread_control[i].mutex);
    if (status) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al desbloquear el lock "
                      "del thread de número %d\n", status, i);
      return status;
    }

    /* Hacer join del thread para que el proceso principal espere a que
       termine */
    status = pthread_join(servidor.thread_pool[i], NULL);
    if(status){
      fprintf(stderr, "[Servidor Prethreaded] Error %d al crear hacer join"
                      " del thread número %d\n", status, i);
      return status;
    }

    status = pthread_mutex_destroy(&servidor.thread_control[i].mutex);
    if(status){
      fprintf(stderr, "[Servidor Prethreaded] Error %d al destruir el mutex"
                       " del thread número %d\n", status, i);
      return status;
    }

    status = pthread_cond_destroy(&servidor.thread_control[i].signal);
    if(status) {
      fprintf(stderr, "[Servidor Prethreaded] Error %d al destruir el cond del"
                      " thread número %d\n", status, i);
      return status;
    }

    }

    // Liberar thread control y thread pool
    free(servidor.thread_control);
    free(servidor.thread_pool);

  return EXIT_SUCCESS;
}
