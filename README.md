# Proyecto 2: Diseño de un *Web Server* Simplificado

El propósito de este proyecto es contrastar varias posibilidades de diseño de un servidor en un ambiente distribuido.
La solución del problema es la implementación de 1 cliente 5 versiones diferentes de *Web Server*: secuencial, forked, threaded, 
pre-forked y pre-threaded. Cada uno se ejecuta de manera independiente de los otros.

### Estructura y compilación del proyecto
- include: Contiene los archivos .h de los 5 *Web Server*, el cliente y util.
- src: Contiene los archivos .c  de los 5 *Web Server*, el cliente y util.
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

## Autores

- Daniel Alvarado Chou.
- Jorge Bolaños Solís.
- Alonso Mondal Durán.
- Kenneth Paniagua Díaz.


Este proyecto se puede encontrar en https://github.com/soatec/proyecto2
