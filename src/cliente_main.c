#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "cliente.h"

/*
 * Rutina para solicitar archivos al servidor
 */
void *request_file(void *args) {
  int status;
  cliente_t *cliente = (cliente_t *)args;

  for(int i = 0; i < cliente->ciclos; i++) {
    // Obtener file descriptor del socket
    status = client_init(cliente);
    if (status) continue;

    // Solicitar archivo
    client_run(cliente);
  }

  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int   opt;
  char *maquina = NULL;
  char *archivo = NULL;
  int   puerto  = -1;
  int   threads = -1;
  int   ciclos  = -1;
  int status = 0;
  cliente_t cliente;
  struct timeval tiempo_inicio, tiempo_final, tiempo_transcurrido;
  pthread_t current_thread;

  while ((opt = getopt(argc, argv, "m:p:a:t:c:")) != -1) {
      switch (opt) {
          case 'm':
              maquina = optarg;
              break;
          case 'p':
              puerto = atoi(optarg);
              break;
          case 'a':
              archivo = optarg;
              break;
          case 't':
              threads = atoi(optarg);
              break;
          case 'c':
              ciclos = atoi(optarg);
              break;
          default:
              fprintf(stderr, "Uso: %s -m nombre_ip_maquina_servidor -p puerto"
                      " -a archivo_solicitado -t número_threads"
                      " -c número_ciclos\n", argv[0]);
              return EXIT_FAILURE;
      }
  }

  if (maquina == NULL) {
      fprintf(stderr, "-m nombre_ip_maquina_servidor es un parámetro obligatorio\n");
      return EXIT_FAILURE;
  }

  if (archivo == NULL) {
      fprintf(stderr, "-a archivo_solicitado es un parámetro obligatorio\n");
      return EXIT_FAILURE;
  }

  if (puerto == -1) {
      fprintf(stderr, "-p puerto es un parámetro obligatorio\n");
      return EXIT_FAILURE;
  } else if (puerto < 0) {
      fprintf(stderr, "-p puerto debe ser positivo\n");
      return EXIT_FAILURE;
  }

  if (threads == -1) {
      fprintf(stderr, "-t número_threads es un parámetro obligatorio\n");
      return EXIT_FAILURE;
  } else if (threads < 0) {
      fprintf(stderr, "-t número_threads debe ser positivo\n");
      return EXIT_FAILURE;
  }

  if (ciclos == -1) {
      fprintf(stderr, "-c número_ciclos es un parámetro obligatorio\n");
      return EXIT_FAILURE;
  } else if (ciclos < 0) {
      fprintf(stderr, "-c número_ciclos debe ser positivo\n");
      return EXIT_FAILURE;
  }

  printf("\n\n  ______   __  __                       __\n");
  printf(" /      \\ |  \\|  \\                     |  \\\n");
  printf("|  $$$$$$\\| $$ \\$$  ______   _______  _| $$_     ______\n");
  printf("| $$   \\$$| $$|  \\ /      \\ |       \\|   $$ \\   /      \\\n");
  printf("| $$      | $$| $$|  $$$$$$\\| $$$$$$$\\\\$$$$$$  |  $$$$$$\\\n");
  printf("| $$   __ | $$| $$| $$    $$| $$  | $$ | $$ __ | $$    $$\n");
  printf("| $$__/  \\| $$| $$| $$$$$$$$| $$  | $$ | $$|  \\| $$$$$$$$\n");
  printf(" \\$$    $$| $$| $$ \\$$     \\| $$  | $$  \\$$  $$ \\$$     \\\n");
  printf("  \\$$$$$$  \\$$ \\$$  \\$$$$$$$ \\$$   \\$$   \\$$$$   \\$$$$$$$\n\n");

  // Inicializar variables compartidas y mutexes
  cliente.ip                 = maquina;
  cliente.puerto             = puerto;
  cliente.archivo            = archivo;
  cliente.threads            = threads;
  cliente.ciclos             = ciclos;
  cliente.archivos_recibidos = 0;
  cliente.errores            = 0;
  cliente.bytes_recibidos    = 0;

  status = pthread_mutex_init(&cliente.mutex_socket, NULL);
  if(status){
    fprintf(stderr, "[Cliente] Error %d al iniciar el mutex del"
                     " socket\n", status);
    return EXIT_FAILURE;
  }


  status = pthread_mutex_init(&cliente.mutex_errores, NULL);
  if(status){
    fprintf(stderr, "[Cliente] Error %d al iniciar el mutex del"
                     " número de errores\n", status);
    return EXIT_FAILURE;
  }

  status = pthread_mutex_init(&cliente.mutex_archivos, NULL);
  if(status){
    fprintf(stderr, "[Cliente] Error %d al iniciar el mutex del"
                     " número de archivos\n", status);
    return EXIT_FAILURE;
  }

  printf("%s ejecutando %d threads que solicitan el archivo %s %d veces "
         "al servidor %s en el puerto %d\n", argv[0], threads, archivo, ciclos,
         maquina, puerto);

  status = gettimeofday(&tiempo_inicio, NULL);
  if (status < 0) {
    fprintf(stderr, "Error %d al obtener el tiempo de inicio\n", errno);
  }

  for (int thread_cnt = 0; thread_cnt < cliente.threads; thread_cnt++) {
    status = pthread_create(&current_thread, NULL, request_file,
                            &cliente);
    if(status){
      fprintf(stderr, "[Cliente] Error %d al crear thread número %d\n",
              status, thread_cnt);
      continue;
    }
    status = pthread_join(current_thread, NULL);
    if(status){
      fprintf(stderr, "[Cliente] Error %d al hacer join del thread número %d\n",
              status, thread_cnt);
    }
  }


  status = gettimeofday(&tiempo_final, NULL);
  if (status < 0) {
    fprintf(stderr, "Error %d al obtener el tiempo de inicio\n", errno);
  }

  status = pthread_mutex_destroy(&cliente.mutex_socket);
  if(status){
    fprintf(stderr, "[Cliente] Error %d al destruir el mutex del"
                     " socket\n", status);
    return EXIT_FAILURE;
  }

  status = pthread_mutex_destroy(&cliente.mutex_errores);
  if(status){
    fprintf(stderr, "[Cliente] Error %d al destruir el mutex del"
                     " número de errores\n", status);
    return EXIT_FAILURE;
  }

  status = pthread_mutex_destroy(&cliente.mutex_archivos);
  if(status){
    fprintf(stderr, "[Cliente] Error %d al destruir el mutex del"
                    " número de archivos\n", status);
    return EXIT_FAILURE;
  }

  timersub(&tiempo_final, &tiempo_inicio, &tiempo_transcurrido);

  printf("\n************************************\n");
  printf("\nEl cliente ha terminado su ejecución\n");
  printf("\n************************************\n\n");

  printf("Tiempo tiempo_transcurrido: %ld s y %ld us\n",
         (long int)tiempo_transcurrido.tv_sec,
         (long int)tiempo_transcurrido.tv_usec/1000);
  printf("Archivos recibidos: %d\n", cliente.archivos_recibidos);
  printf("Bytes recibidos: %d\n",    cliente.bytes_recibidos);
  printf("Errores reportados: %d\n", cliente.errores);

  return EXIT_SUCCESS;
}
