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