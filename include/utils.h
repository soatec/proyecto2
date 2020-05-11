#ifndef TAREA2_UTILS_H
#define TAREA2_UTILS_H

#include <signal.h>
#include <stdint.h>
#include <stdbool.h>

/**
 *
 * @param root
 * @param file_descriptor
 * @param server_name
 * @return
 */
int respond_to_request(char *root, int file_descriptor, char *server_name);

/**
 *
 * @param file_descriptor
 * @param server_name
 * @return
 */
int respond_internal_server_error(int file_descriptor, char *server_name);

/**
 *
 * @param file_descriptor
 * @param server_name
 * @return
 */
int respond_service_unavailable(int file_descriptor, char *server_name);

/** Util para inicializar un socket TCP
 *
 * @param [in] puerto       es el número de puerto para asociar al socket
 * @param [in] direccion_ip es la dirección IP para asociar al socket, si es
 *                          NULL, se asume 127.0.0.1
 * @param[in] servidor      es una bandera que indica si la conexión es para un
 *                          servidor (true) o un cliente (false)
 *
 * @returns file descriptor del socket escuchando en la dirección y puerto
 *          dados, -1 en caso de error.
 */
int tcp_connection_init(uint16_t puerto, char *direccion_ip, bool servidor);

/** Util para desinicializar un socket TCP
 *
 * @param [in] fd es file descriptor del socket
 *
 * @returns 0 en caso de éxito, -1 en caso de error
 */
int tcp_connection_uninit(int fd);

/** Util para enviar un HTTP request
 *
 * @param [in] file_descriptor es file descriptor del socket
 * @param [in] file_location   es el URI del recurso a solicitar
 * @param [in] times           es la cantidad de veces que se solicitará el
 *                             archivo
 *
 * @returns 0 en caso de éxito, -1 en caso de error
 */
int send_get_request(int file_descriptor, char *file_location, int times);

#endif //TAREA2_UTILS_H
