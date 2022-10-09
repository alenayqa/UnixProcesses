#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socketutils.h"

int main(int argc, char **argv)
{

    // Названия файлов с матрицей и вектором подаются через аргументы командной строки
    if (argc < 3)
    {
        printf("matrix and vector files required\n");
        exit(1);
    }
    char *matrix_fname = argv[1];
    char *vector_fname = argv[2];
    
    double *vector = NULL;
    double *flatten_matrix = NULL;

    int vector_size = read_from_file(vector_fname, &vector);
    if (vector_size == -1)
    {
        printf("cannot open file %s\n", vector_fname);
        exit(1);
    }

    int flatten_matrix_size = read_from_file(matrix_fname, &flatten_matrix);
    if (flatten_matrix_size == -1)
    {
        printf("cannot open file %s\n", matrix_fname);
        exit(1);
    }

    for (int i = 0; i < vector_size; i++)
    {
        printf("%lf\n", vector[i]);
    }
    free(vector);
    
    return 0;
}