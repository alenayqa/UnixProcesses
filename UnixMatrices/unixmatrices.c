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
    int vector_size = read_from_file(vector_fname, &vector);

    for (int i = 0; i < vector_size; i++)
    {
        printf("%lf\n", vector[i]);
    }
    free(vector);
    
    return 0;
}