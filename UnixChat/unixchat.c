#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include "shmutils.h"

int this_user_index;
int users_shared_memory_id;
int *users;
int semid;

int msg_shared_memory_id;
char *msg;

void on_interrupt(int sig);

void on_message_received(int sig);




int main(int argc, char *argv[])
{
     int user_pid = getpid();

     // Обработка закрытия через Ctrl-C
     signal(SIGINT, on_interrupt);

     // Обработка закрытия через Ctrl-Z
     signal(SIGTSTP, on_interrupt);

     // Аварийное завершение
     signal(SIGABRT, on_interrupt);

     // Прием сообщения
     signal(SIGUSR1, on_message_received);
     
     // Создание массива семафоров
     semid = semget(2015, MAX_USERS, IPC_CREAT | 0666);
     printf("SEM %d\n", semid);
     // Подключение к разделяемой памяти с пользователями
     users_shared_memory_id = users_shared_memory_getter(semid);

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

     // Добавление пользователя в общий список и выдача ему номера
     if (users[0] < MAX_USERS)
     {
          this_user_index = append_user(user_pid, users);
     }
     else
     {
          shmdt(users);
          shmdt(msg);
          printf("chatroom is full\n");
          exit(0);
     }
     printf("users online: %d\n", users[0]);
     printf("SEM INFO start %d %d\n",semctl(semid, this_user_index, GETVAL), this_user_index);
     
     char *new_msg;
     size_t len = 0;
     ssize_t line_size;
     while (1)
     {
          // Считывание сообщения
          line_size = getline(&new_msg, &len, stdin);

          // Если длина сообщения превысила допустимый максимум
          if (line_size > MAX_MSGLEN)
          {
               printf("\n!! MESSAGE IS TOO LONG !!\n\n");
               continue;
          }

          // Ожидание остальных процессов, пока они не прочитают сообщение
          for (int i = 1; i < MAX_USERS; i++)
          {
               if (users[i]==-1 || i==this_user_index) continue;
               struct sembuf sem_wait;
	          sem_wait.sem_num = i;
	          sem_wait.sem_op = 0;
	          sem_wait.sem_flg = 0;
               semop(semid, &sem_wait, 1);
          }

          // Запись сообщения в разделяемую память
          strcpy(msg, new_msg);

          // Отправка сообщения остальным пользователям
          send_msg(users, this_user_index, semid);
     }

}



void on_interrupt(int sig)
{
     int close = user_exit(users_shared_memory_id, users, this_user_index, msg_shared_memory_id, msg);
     if (close == CLOSE_SHARED_MEMORY_SUCCESS)
     {
          semctl(semid, IPC_RMID, 0);
          printf("!! close shared memory !!\n");
     }
     else if (close == CLOSE_SHARED_MEMORY_ERROR)
     {
          printf("could not close shared memory\n");
     }
     printf("bye!\n");
     exit(0);
}

void on_message_received(int sig)
{
     signal(sig, SIG_IGN);
     // Освобождение семафора - сигнал о том, что процесс прочитал сообщение
     struct sembuf sem_release;
	sem_release.sem_num = this_user_index;
	sem_release.sem_op = -1;
	sem_release.sem_flg = 0;
     semop(semid, &sem_release, 1);

     printf("%s", msg);
     signal(sig, on_message_received);
}