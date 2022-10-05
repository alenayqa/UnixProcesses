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

int msg_shared_memory_id;
char *msg;

void on_keyboard_interrupt(int sig);

void on_message_received(int sig);




int main(int argc, char *argv[])
{
     int _pid = getpid();
     printf("%d\n", _pid);
     this_user_index = 1;

     // Обработка закрытия через Ctrl-C
     signal(SIGINT, on_keyboard_interrupt);

     // Обработка закрытия через Ctrl-Z
     signal(SIGTSTP, on_keyboard_interrupt);

     
     // Подключение к разделяемой памяти с пользователями
     users_shared_memory_id = users_shared_memory_getter();
     if (users_shared_memory_id == -1 || (users = (int *) shmat(users_shared_memory_id, 0, 0)) == NULL)
     {
          printf("couldn't get shared memory\n");
          exit(1);
     }

     // Подключение к разделяемой памяти с сообщениями
     msg_shared_memory_id = msg_shared_memory_getter();
     if (msg_shared_memory_id == -1 || (msg = (char *)shmat(msg_shared_memory_id, 0, 0)) == NULL)
     {
          printf("couldn't get shared memory\n");
          exit(1);
     }

     users[0]++;
     printf("users online: %d\n", users[0]);

     while (1)
     {
          
     }
}



void on_keyboard_interrupt(int sig)
{
     int close = user_exit(users_shared_memory_id, users, this_user_index, msg_shared_memory_id, msg);
     if (close == CLOSE_SHARED_MEMORY_SUCCESS)
     {
          printf("!! close shared memory !!\n");
     }
     else if (close == CLOSE_SHARED_MEMORY_ERROR)
     {
          printf("could not close shared memory");
     }
     printf("bye!\n");
     exit(0);
}

void on_message_received(int sig)
{

}