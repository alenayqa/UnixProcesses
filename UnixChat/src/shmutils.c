#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shmutils.h"
#include "message.h"

int shared_memory_getter()
{
    int id;
    // Получение доступа БЕЗ СОЗДАНИЯ
    if ((id = shmget(USERS_SHARED_MEMORY_KEY, (MAX_USERS + 1) * sizeof(int), PERMS)) < 0)
    {
        /*
        * ===============================================
         *   ** ПАМЯТЬ НЕ ПОЛУЧЕНА **
         * 
         *   Теперь уже создаем разделяемую память
         *   Заполняем сегмент начальными значениями
         *   Если и в этом случае произошла ошибка,
         *   возвращаем -1
         * 
        * ===============================================
        */
        if ((id = shmget(USERS_SHARED_MEMORY_KEY, (MAX_USERS + 1) * sizeof(int), PERMS | IPC_CREAT)) < 0)
        {
            // При ошибке возвращаем -1
            return -1;
        }

        // Заполнение сегмента начальными значениями
    }
    return id;
}