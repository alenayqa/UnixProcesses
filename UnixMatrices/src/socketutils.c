#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socketutils.h"

int read_from_file(char *fname, double **buf)
{
    double x;
    // Зарезервированный размер буфера
    int size = 8;
    // Текущее количество элементов
    int n = 0;

    FILE *f;
    f = fopen(fname, "r");

    if (!f)
    {
        return -1;
    }

    *buf = malloc(size*sizeof(double));

    // Читаем, пока не дошли до конца файла
    while (fscanf(f, "%lf", &x)!=EOF)
    {
        // Перевыделяем память, если её не хватает для записи нового числа
        if (n >= size)
        {
            size = size*2;
            *buf = realloc(*buf, size*sizeof(double));
        }
        // Записываем в буфер считанное значение
        *(*buf + n) = x;
        n++;
    }

    fclose(f);
    return n;
}

double** create_matrix(int flatten_matrix_size, int vector_size, double* flatten_matrix, int *rows, int *cols)
{
    // Проверка корректности размерности (возможно ли создать матрицу)
    if (flatten_matrix_size % vector_size != 0)
    {
        return NULL;
    }
    *cols = vector_size;
    *rows = flatten_matrix_size / vector_size;

    // Выделение памяти под двумерный массив и его заполнение
    double **matrix = malloc(*rows * sizeof(double*));
    int src_index;
    for (int i = 0; i < *rows; i++)
    {
        matrix[i] = malloc(*cols * sizeof(double));
        for (int j =  0; j < *cols; j++)
        {
            matrix[i][j] = flatten_matrix[src_index++];
        }
    }

    return matrix;
}
