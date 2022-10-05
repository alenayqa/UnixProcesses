#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include "shmutils.h"

int this_user_index;
int users_shared_memory_id;
int *users;

void on_keyboard_interrupt(int sig);




int main(int argc, char *argv[])
{
     int _pid = getpid();
     printf("%d\n", _pid);
     this_user_index = 1;

     // Обработка закрытия через Ctrl-C
     signal(SIGINT, on_keyboard_interrupt);
     
     users_shared_memory_id = shared_memory_getter();
     if (users_shared_memory_id == -1 || (users = (int *) shmat(users_shared_memory_id, 0, 0)) == NULL)
     {
          printf("couldn't get shared memory");
          exit(1);
     }

     users[0]++;
     printf("ALL USERS: %d\n", users[0]);

     while (1)
     {
          
     }
}

/*
* ===============================================
 *   ** ОТКЛЮЧЕНИЕ ПОЛЬЗОВАТЕЛЯ **
 * 
 *   При отключении нужно уменьшить число пользователей
 *   (users[0]) на единицу
 *   Также присваиваем соответствующей ячейке
 *   (users[this_user_index]) значение -1
* ===============================================
*/

void on_keyboard_interrupt(int sig)
{
     users[0]--;
     users[this_user_index] = -1;
     printf("ALL USERS: %d\n", users[0]);
     if (users[0]==0)
     {
          shmdt(users);
          if (shmctl (users_shared_memory_id, IPC_RMID, (struct shmid_ds *) 0) < 0)
          {
               printf("shared memory remove error");
          }
     }
     else
     {
          shmdt(users);
     }
     printf("bye!\n");
     exit(0);
}