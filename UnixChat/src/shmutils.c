#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>
#include "shmutils.h"

int users_shared_memory_getter()
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
       printf("create shared memory\n");

        if ((id = shmget(USERS_SHARED_MEMORY_KEY, (MAX_USERS + 1) * sizeof(int), PERMS | IPC_CREAT)) < 0)
        {
            // При ошибке возвращаем -1
            return -1;
        }

        // Заполнение сегмента начальными значениями
        int* users;
        if ((users = (int *) shmat(id, 0, 0)) == NULL)
        {
            // При ошибке возвращаем -1
            return -1;
        }
        users[0] = 0;

        // Нумерация начинается с единицы
        for (int i = 1; i <= MAX_USERS; i++)
        {
            users[i] = -1;
        }
        shmdt(users);
    }
    return id;
}

int append_user(int pid, int *users)
{
    for (int i = 1; i <= MAX_USERS; i++)
    {
        if (users[i]==-1)
        {
            users[0]++;
            users[i] = pid;
            return i;
        }
    }
}

int msg_shared_memory_getter()
{
    return shmget(MSG_SHARED_MEMORY_KEY, MAX_MSGLEN * sizeof(char), PERMS | IPC_CREAT);
}

void send_msg(int *users, int user_index)
{
    for (int i = 1, count = 0; i<=MAX_USERS && count<users[0]; i++)
    {
        if (users[i]!=-1 && i!=user_index)
        {
            kill(users[i], SIGUSR1);
            count++;
        }
    }
}

int user_exit(int users_shared_memory_id, int *users, int user_index, int msg_shared_memory_id, char *msg)
{
    users[0]--;
    users[user_index] = -1;
    printf("\nusers online: %d\n", users[0]);
    if (users[0]==0)
    {
        shmdt(users);
        shmdt(msg);
        int close_memory = shmctl (users_shared_memory_id, IPC_RMID, (struct shmid_ds *) 0) < 0 &&
                            shmctl(msg_shared_memory_id, IPC_RMID, (struct shmid_ds *) 0) < 0;
        if (close_memory)
        {
            return CLOSE_SHARED_MEMORY_ERROR;
        }
        else
        {
            return CLOSE_SHARED_MEMORY_SUCCESS;
        }
    }
    else
    {
        shmdt(users);
        shmdt(msg);
    }
    return DETACH_SUCCESS;
}


