# Proyecto 2: Diseño de un *Web Server* Simplificado

El propósito de este proyecto es contrastar varias posibilidades de diseño de un servidor en un ambiente distribuido.
La solución del problema es la implementación de 1 cliente y 5 versiones diferentes de *Web Server*: secuencial, forked, threaded, pre-forked y pre-threaded. Cada uno se ejecuta de manera independiente de los otros e implementa de forma básica los métodos HEAD, GET, PUT y DELETE. Para finalizar cualquier servidor de manera controlada es necesario presionar Ctrl + C.

### Estructura y compilación del proyecto
- include: Contiene los archivos .h de los 5 *Web Server*, el cliente y utils.
- src: Contiene los archivos .c  de los 5 *Web Server*, el cliente y utils.
- build: contiene los archivos ejecutables generados por el makefile. Este directorio se crea con la regla `all` del makefile.

Para compilar el proyecto, se utiliza el makefile:
```
make
```

## Correr el proyecto
Una vez que compila el proyecto se crea seis ejecutables.

### *Web Server* secuencial
Recibe como parámetros -p puerto -r root_servidor

```
./build/secuencial -p [puerto] -r [root_servidor]

```

### *Web Server* forked

Recibe como parámetros -p puerto -r root_servidor

```
./build/forked -p [puerto] -r [root_servidor]

```

### *Web Server* threaded

Recibe como parámetros -p puerto -r root_servidor

```
./build/threaded -p [puerto] -r [root_servidor]

```


### *Web Server* pre-forked

Recibe como parámetros -p puerto -r root_servidor -n número_procesos

```
./build/preforked -p [puerto] -r [root_servidor] -n [número_procesos]

```

### *Web Server* pre-threaded

Recibe como parámetros -p puerto -r root_servidor -n número_threads

```
./build/prethreaded -p [puerto] -r [root_servidor] -n [número_threads]

```

### Cliente

Recibe como parámetros -m nombre_ip_maquina_servidor -p puerto-a archivo_solicitado -t número_threads -c número_ciclos

```
./build/cliente -m [nombre_ip_maquina_servidor] -p [puerto] -a [archivo_solicitado] -t [número_threads] -c [número_ciclos]

```
## Ejemplo de uso

En este ejemplo se usa el generador de *HTTP requests* online https://reqbin.com/ usando Google Chrome Version 81.0.4044.113.

### Inicializar un servidor
Se usa el servidor forked con root `/home/user/server_root` en el puerto 10000, 
ejecutando en la raíz del proyecto:

```
make
./build/forked -r "/home/user/server_root" -p 10000
```

En cada caso, la imagen muestra del lado izquierdo el *request* enviado y el lado derecho son las respuestas del *Web Server*.

![Alt text](docs/serverinit.png?raw=true "Title")

### Ejecutar PUT

![Alt text](docs/put.png?raw=true "Title")

### Ejecutar GET

![Alt text](docs/get.png?raw=true "Title")

### Ejecutar HEAD

![Alt text](docs/head.png?raw=true "Title")

### Ejecutar DELETE

![Alt text](docs/delete.png?raw=true "Title")

## Autores

- Greylin Arias Montiel.
- Daniel Alvarado Chou.
- Jorge Bolaños Solís.
- Alonso Mondal Durán.
- Kenneth Paniagua Díaz.


Este proyecto se puede encontrar en https://github.com/soatec/proyecto2
