#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socketutils.h"

double *vector;
double *flatten_matrix;
double **matrix;
int rows=0, cols=0;

void free_memory()
{
    free(vector);
    free(flatten_matrix);
    for (int i = 0; i < rows; i++)
        free(matrix[i]);
    free(matrix);
}

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
    matrix =  create_matrix(flatten_matrix_size, vector_size, flatten_matrix, &rows, &cols);

    if (!matrix)
    {
        printf("invalid matrix size, must be N x %d\n", vector_size);
        free_memory();
        exit(1);
    }

    FILE* f;
    f = fopen("output.txt", "w");
    for (int i = 0; i < rows; i++)
    {
        fprintf(f, "%lf\n", dot(cols, matrix[i], vector));
    }
    fclose(f);

    free_memory();
    
    return 0;
}