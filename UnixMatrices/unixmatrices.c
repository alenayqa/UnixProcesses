#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include "socketutils.h"


pthread_t server_phtread;


// Вектор, на который умножается матрица
double *vector;
// Результат
double *result_vector;
// "Выпрямленная матрица"
double *flatten_matrix;
// Матрица
double **matrix;
// Размерности матриц
int rows=0, cols=0;
// Текущее число работающих процессов
int num_processes = 0;
// Сокет
int server_socket;
// Очередная строка матрицы, которую надо умножить на вектор
int current_row=0;
// Число строк матрицы, которые осталось умножить на вектор
int complete = 0;


pthread_mutex_t multiply_mutex;
pthread_mutex_t num_processes_mutex;
pthread_mutex_t complete_mutex;

void free_memory();

void on_interrupt(int sig);

void on_sigpipe(int sig);


void write_result()
{
    FILE* f;
    f = fopen("output.txt", "w");
    for (int i = 0; i < rows; i++)
    {
        fprintf(f, "%lf\n", result_vector[i]);
    }
    fclose(f);
}

void *client_thread(void *vargp)
{
    int client_socket = *((int *)vargp);
    int this_process_num;
    int msg;

    pthread_mutex_lock(&num_processes_mutex);
    this_process_num = ++num_processes;
    pthread_mutex_unlock(&num_processes_mutex);

    // Отправка клиенту начальных данных - вектор и его размер
    if (send(client_socket, &cols, sizeof(int), 0) == -1) pthread_exit(0);
    send(client_socket, vector, cols*sizeof(double), 0);

    pthread_mutex_lock(&complete_mutex);
    if (complete == 0)
    {
        msg = END_MSG;
        send(client_socket, &msg, sizeof(int), 0);
    }
    pthread_mutex_unlock(&complete_mutex);

    int local_current_row;
    double result;

    while (1)
    {
        // Получение строки, которую надо умножить на вектор
        pthread_mutex_lock(&multiply_mutex);
        local_current_row = current_row;
        if (current_row < rows)
            current_row++;
        pthread_mutex_unlock(&multiply_mutex);


        // Если считать уже не нужно
        if (local_current_row == rows)
        {
            msg = END_MSG;
            send(client_socket, &msg, sizeof(int), 0);
            break;
        }

        // Отправка строки матрицы на вычисление клиенту и получение результата
        msg = CALC_MSG;
        printf("Process %d started work with row %d!\n", this_process_num, local_current_row);
        send(client_socket, &msg, sizeof(int), 0);
        send(client_socket, matrix[local_current_row], cols*sizeof(double), 0);
        recv(client_socket, &result, sizeof(double), 0);
        result_vector[local_current_row] = result;

        pthread_mutex_lock(&complete_mutex);
        complete--;
        pthread_mutex_unlock(&complete_mutex);

    }

}

void *server_thread(void *vargp)
{
    while (1)
    {

        // При подключении нового пользователя выделяем ему поток
        int client_socket = accept(server_socket, NULL, NULL);
        pthread_t thread_id;

        pthread_create(&thread_id, NULL, client_thread, (void*)&client_socket);
    }
}

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
    // Выделение памяти под результирующий вектор
    result_vector = malloc(rows * sizeof(double));

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

    struct sockaddr_in server_address;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        printf("SOCKET ERROR\n");
        exit(0);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(MATRIX_PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int A = 1;
    // Сокет может "висеть" в системе, освобождаем его
    // https://handsonnetworkprogramming.com/articles/bind-error-98-eaddrinuse-10048-wsaeaddrinuse-address-already-in-use/
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&A, sizeof(A));
    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0)
    {
        printf("BIND ERROR\n");
        exit(0);
    }
    if (listen(server_socket, 5) < 0)
    {
        printf("LISTEN ERROR\n");
        exit(0);
    }


    // Создание мьютексов для синхронизации
    pthread_mutex_init(&multiply_mutex, NULL);
    pthread_mutex_init(&num_processes_mutex, NULL);
    pthread_mutex_init(&complete_mutex, NULL);

    complete = rows;

    pthread_create(&server_phtread, NULL, server_thread, NULL);

    while (1)
    {
        // Отлавливаем момент, когда все строки матрицы были умножены на вектор
        pthread_mutex_lock(&complete_mutex);
        if (complete==0)
        {
            write_result();
            free_memory();
            pthread_mutex_unlock(&complete_mutex);
            break;
        }
        pthread_mutex_unlock(&complete_mutex);
        sleep(1);
    }

    pthread_mutex_destroy(&multiply_mutex);
    pthread_mutex_destroy(&num_processes_mutex);
    pthread_mutex_destroy(&complete_mutex);
    
    
    return 0;
}

void free_memory()
{
    free(vector);
    free(flatten_matrix);
    free(matrix);
    close(server_socket);
    printf("EXIT\n");

}

void on_interrupt(int sig)
{
    free_memory();
    printf("bye!\n");
    exit(0);
}
