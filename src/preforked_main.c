#include <stdio.h>            // input/output library
#include <unistd.h>           // misc. UNIX functions
#include <stdlib.h>           // standard library
#include <string.h>           // string library
#include <termios.h>
#include <dirent.h>


#include <sys/types.h>        // socket types plus various type definitions
#include <sys/socket.h>       // socket definitions
#include <arpa/inet.h>        // inet (3) funtions
#include <fcntl.h>            // for O_* constants
#include <errno.h>            // error number library
#include <sys/sendfile.h>     // more constants
#include <sys/stat.h>         // more constants

#include <signal.h>           // signal handling
#include <sys/mman.h>         // mmap library
#include <pthread.h>
#include <sys/wait.h>

#include "preforked.h"

#define SERVER_BACKLOG 100


// Structure to hold variables that will be placed in shared memory
typedef struct {
    pthread_mutex_t mutexlock;
    pthread_mutex_t accept_connection_lock;
    int totalbytes;
} sharedVariables;

int list_s;                   // listening socket

//#############################################################################################

// Increment the global count of data sent out 
int recordTotalBytes(int bytes_sent, sharedVariables *mempointer)
{
    // Lock the mutex
    pthread_mutex_lock(&(*mempointer).mutexlock);
    // Increment bytes_sent
    (*mempointer).totalbytes += bytes_sent;
    // Unlock the mutex
    pthread_mutex_unlock(&(*mempointer).mutexlock);
    // Return the new byte count
    return (*mempointer).totalbytes;
}

// clean up listening socket on ctrl-c
void cleanup(int sig)
{
    printf("PID:%i Cleaning up connections and exiting.\n", getpid());

    // try to close the listening socket
    if (close(list_s) < 0)
    {
        fprintf(stderr, "Error calling close()\n");
        exit(EXIT_FAILURE);
    }

    // Close the shared memory we used
    shm_unlink("/sharedmem");

    // exit with success
    exit(EXIT_SUCCESS);
}

int mygetch(void)
{
    int ch;
    struct termios oldt, newt;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

int main(int argc, char *argv[])
{
    int opt;
    char *root = NULL;
    int puerto = -1;
    int procesos = -1;

    while ((opt = getopt(argc, argv, "p:r:n:")) != -1)
    {
        switch (opt)
        {
            case 'p':
                puerto = atoi(optarg);
                break;
            case 'r':
                root = optarg;
                break;
            case 'n':
                procesos = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Uso: %s -p puerto -r root_servidor "
                    "-n número_procesos\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (root == NULL)
    {
        fprintf(stderr, "-r root_servidor es un parámetro obligatorio\n");
        return EXIT_FAILURE;
    }

    DIR * dir;
    if ((dir = opendir(root)))
    {
        closedir(dir);
    }
    else if (ENOENT == errno)
    {
        fprintf(stderr, "-r root_servidor no es un directorio valido\n");
        return EXIT_FAILURE;
    }
    else
    {
        fprintf(stderr, "-r root_servidor no es un directorio valido\n");
        return EXIT_FAILURE;
    }

    if (puerto == -1)
    {
        fprintf(stderr, "-p puerto es un parámetro obligatorio\n");
        return EXIT_FAILURE;
    }
    else if (puerto < 0)
    {
        fprintf(stderr, "-p puerto debe ser positivo\n");
        return EXIT_FAILURE;
    }

    if (procesos == -1)
    {
        fprintf(stderr, "-n número_procesos es un parámetro obligatorio\n");
        return EXIT_FAILURE;
    }
    else if (procesos < 0)
    {
        fprintf(stderr, "-n número_procesos debe ser positivo\n");
        return EXIT_FAILURE;
    }

    printf("\n\n  ______                                 __        __                            _______                        ______                      __                        __\n");
    printf(" /      \\                               |  \\      |  \\                          |       \\                      /      \\                    |  \\                      |  \\\n");
    printf("|  $$$$$$\\  ______    ______  __     __  \\$$  ____| $$  ______    ______        | $$$$$$$\\  ______    ______  |  $$$$$$\\ ______    ______  | $$   __   ______    ____| $$\n");
    printf("| $$___\\$$ /      \\  /      \\|  \\   /  \\|  \\ /      $$ /      \\  /      \\       | $$__/ $$ /      \\  /      \\ | $$_  \\$$/      \\  /      \\ | $$  /  \\ /      \\  /      $$\n");
    printf(" \\$$    \\ |  $$$$$$\\|  $$$$$$\\\\$$\\ /  $$| $$|  $$$$$$$|  $$$$$$\\|  $$$$$$\\      | $$    $$|  $$$$$$\\|  $$$$$$\\| $$ \\   |  $$$$$$\\|  $$$$$$\\| $$_/  $$|  $$$$$$\\|  $$$$$$$\n");
    printf(" _\\$$$$$$\\| $$    $$| $$   \\$$ \\$$\\  $$ | $$| $$  | $$| $$  | $$| $$   \\$$      | $$$$$$$ | $$   \\$$| $$    $$| $$$$   | $$  | $$| $$   \\$$| $$   $$ | $$    $$| $$  | $$\n");
    printf("|  \\__| $$| $$$$$$$$| $$        \\$$ $$  | $$| $$__| $$| $$__/ $$| $$            | $$      | $$      | $$$$$$$$| $$     | $$__/ $$| $$      | $$$$$$\\ | $$$$$$$$| $$__| $$\n");
    printf(" \\$$    $$ \\$$     \\| $$         \\$$$   | $$ \\$$    $$ \\$$    $$| $$            | $$      | $$       \\$$     \\| $$      \\$$    $$| $$      | $$  \\$$\\ \\$$     \\ \\$$    $$\n");
    printf("  \\$$$$$$   \\$$$$$$$ \\$$          \\$     \\$$  \\$$$$$$$  \\$$$$$$  \\$$             \\$$       \\$$        \\$$$$$$$ \\$$       \\$$$$$$  \\$$       \\$$   \\$$  \\$$$$$$$  \\$$$$$$$\n\n");

    printf("%s ejecutando con un máximo de %d procesos con el root path %s "
        "en el puerto %d\n", argv[0], procesos, root, puerto);

    int conn_s; //  connection socket
    short int port = puerto;  //  port number
    struct sockaddr_in servaddr;  //  socket address structure

    // set up signal handler for ctrl-c
    (void) signal(SIGINT, cleanup);

    // create the listening socket
    if ((list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Error creating listening socket.\n");
        exit(EXIT_FAILURE);
    }

    // set all bytes in socket address structure to zero, and fill in the relevant data members
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // bind to the socket address

    if (bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        fprintf(stderr, "Error calling bind()\n");
        exit(EXIT_FAILURE);
    }

    // Listen on socket list_s
    if ((listen(list_s, SERVER_BACKLOG)) == -1)
    {
        fprintf(stderr, "Error Listening\n");
        exit(EXIT_FAILURE);
    }

    // Set up some shared memory to store our shared variables in

    // Close the shared memory we use just to be safe
    shm_unlink("/sharedmem");

    int sharedmem;

    // Open the memory
    if ((sharedmem = shm_open("/sharedmem", O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR)) == -1)
    {
        fprintf(stderr, "Error opening sharedmem in main() errno is: %s ", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Set the size of the shared memory to the size of my structure
    ftruncate(sharedmem, sizeof(sharedVariables));

    // Map the shared memory into our address space
    sharedVariables * mempointer;

    // Set mempointer to point at the shared memory
    mempointer = mmap(NULL, sizeof(sharedVariables), PROT_READ | PROT_WRITE, MAP_SHARED, sharedmem, 0);

    // Check the memory allocation went OK
    if (mempointer == MAP_FAILED)
    {
        fprintf(stderr, "Error setting shared memory for sharedVariables in recordTotalBytes() error is %d \n ", errno);
        exit(EXIT_FAILURE);
    }

    // Initalise the mutex
    pthread_mutex_init(&(*mempointer).mutexlock, NULL);
    pthread_mutex_init(&(*mempointer).accept_connection_lock, NULL);
    // Set total bytes sent to 0
    (*mempointer).totalbytes = 0;

    // Size of the address
    int addr_size = sizeof(servaddr);

    // Sizes of data were sending out
    int headersize;
    int pagesize;
    int totaldata;
    // Number of child processes we have spawned
    int children = 0;
    int childref[procesos - 1];
    // Variable to store the ID of the process we get when we spawn
    pid_t pid;

    // Loop infinitly serving requests
    while (1)
    {
        if (children == procesos)
        {
            printf("Press any key to close server.\n");
            mygetch();
            printf("Server Closing...\n");
            int i;
            for (i = 0; i < children; i++)
            {
                kill(childref[i], 2);
            }

            wait(NULL);
            totaldata = recordTotalBytes(0, mempointer);
            fprintf(stderr,"Total bytes sent %d\n", totaldata);
            fprintf(stderr, "Parent ");
            cleanup(2);
            break;
        }

        // If we haven't already spawned 10 children fork
        if (children < procesos)
        {
            pid = fork();
            childref[children] = pid;
            children++;
        }

        // If the pid is -1 the fork failed so handle that
        if (pid == -1)
        {
            fprintf(stderr, "can't fork, error %d\n", errno);
            exit(1);
        }

        // Have the child process deal with the connection
        if (pid == 0)
        {
            // Have the child loop infinetly dealing with a connection then getting the next one in the queue
            while (1)
            {
                // Accept a connection
                pthread_mutex_lock(&(*mempointer).accept_connection_lock);
                conn_s = accept(list_s, (struct sockaddr *) &servaddr, &addr_size);
                pthread_mutex_unlock(&(*mempointer).accept_connection_lock);

                // If something went wrong with accepting the connection deal with it
                if (conn_s == -1)
                {
                    fprintf(stderr, "Error accepting connection \n");
                    exit(1);
                }

                // Get the message from the file descriptor
                char *header = getMessage(conn_s);

                // Parse the request
                httpRequest details = parseRequest(header, root);

                // Free header now were done with it
                free(header);

                // Print out the correct header
                headersize = writeHeader(conn_s, details.returncode, details.filename, details.contentType);

                // Print out the file they wanted
                pagesize = writeFile(conn_s, details.filename);

                // Increment our count of total datasent by all processes and get back the new total
                totaldata = recordTotalBytes(headersize + pagesize, mempointer);

                // Print out which process handled the request and how much data was sent
                printf("[%d] served a request of %d bytes\n", getpid(), headersize + pagesize);

                // Close the connection now were done
                close(conn_s);
            }

            //Make sure fork child exits if it magically escapes the loop
            exit(0);
        }
    }

    return EXIT_SUCCESS;
}
