#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include "socketutils.h"

int n;
int NB;
double* vector;
double* column;
int network_socket;

void free_memory();

void on_interrupt(int sig);

int main(int argc, char **argv)
{
    // Обработка закрытия через Ctrl-C
    signal(SIGINT, on_interrupt);

    // Обработка закрытия через Ctrl-Z
    signal(SIGTSTP, on_interrupt);

    // Аварийное завершение
    signal(SIGABRT, on_interrupt);

    struct sockaddr_in server_address;

    network_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(MATRIX_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    recv(network_socket, &n, sizeof(int), 0);
    vector = malloc(n*sizeof(double));
    column = malloc(n*sizeof(double));
    recv(network_socket, vector, n*sizeof(double), 0);
    for (int i = 0; i < n; i++)
    {
        printf("%lf\n", *(vector + i));
    }

    free_memory();
}

void on_interrupt(int sig)
{
    free_memory();
    exit(0);
}

void free_memory()
{
    free(vector);
    free(column);
    close(network_socket);
}
