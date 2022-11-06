#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "socketutils.h"

double *vector;
double *flatten_matrix;
double **matrix;
int rows=0, cols=0;
int num_processes = 1;
int server_socket;

void *server_thread(void *vargp)
{
    // Отправка клиенту начальных данных - вектор и его размер
    int client_socket = *((int *)vargp);
    send(client_socket, &cols, sizeof(int), 0);
    send(client_socket, vector, cols*sizeof(double), 0);
 
}


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

    // Названия файлов с матрицей и вектором подаются через аргументы командной строки
    if (argc < 3)
    {
        printf("matrix and vector files required\n");
        exit(1);
    }
    char *matrix_fname = argv[1];
    char *vector_fname = argv[2];

    int vector_size = read_from_file(vector_fname, &vector);
    if (vector_size == -1)
    {
        printf("cannot open file %s\n", vector_fname);
        exit(1);
    }

    // Матрица изначально считывается в "выпрямленном" виде, то есть в виде одномерного массива
    // Её размерности будут зависеть от длины считанного вектора
    int flatten_matrix_size = read_from_file(matrix_fname, &flatten_matrix);
    if (flatten_matrix_size == -1)
    {
        printf("cannot open file %s\n", matrix_fname);
        free_memory();
        exit(1);
    }

    // Создание матрицы из линейного массива
    matrix = create_matrix(flatten_matrix_size, vector_size, flatten_matrix, &rows, &cols);

    if (!matrix)
    {
        printf("invalid matrix size, must be N x %d\n", vector_size);
        free_memory();
        exit(1);
    }

    // Количество запускаемых процессов подается через командную строку
    // Если ничего не передано, то будет запускаться только 1 процесс
    if (argc >= 4)
    {
        num_processes = atoi(argv[4]);
        // Не более 8 процессов
        if (num_processes > 8) num_processes = 8;
        // Хотя бы один процесс
        if (num_processes <= 0) num_processes = 1;
        // Не запускать излишние процессы
        if (rows >= 1 && num_processes > rows)
        {
            num_processes = rows;
        }
    }

    struct sockaddr_in server_address;
    int matrix_socket;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(MATRIX_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    listen(server_socket, 5);


    char server_message[256] = "You have reached the server!";
    // int client_socket = accept(server_socket, NULL, NULL);

    while (1)
    {
        // При подключении нового пользователя выделяем ему поток
        int client_socket = accept(server_socket, NULL, NULL);
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, server_thread, (void*)&client_socket);
        // client_socket = accept(server_socket, NULL, NULL);
        // send(client_socket, &cols, sizeof(int), 0);
        // send(client_socket, vector, cols*sizeof(double), 0);

    }
    // FILE* f;
    // f = fopen("output.txt", "w");
    // for (int i = 0; i < rows; i++)
    // {
    //     fprintf(f, "%lf\n", dot(cols, matrix[i], vector));
    // }
    // fclose(f);

    // system("neofetch");

    free_memory();
    
    return 0;
}

void free_memory()
{
    free(vector);
    free(flatten_matrix);
    for (int i = 0; i < rows; i++)
        free(matrix[i]);
    free(matrix);
    close(server_socket);

}

void on_interrupt(int sig)
{
    free_memory();
    printf("bye!\n");
    exit(0);
}