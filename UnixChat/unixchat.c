#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include "shmutils.h"
#include <ncurses.h>
#include <ctype.h>


int this_user_index;
int users_shared_memory_id;
int *users;
int semid;

int msg_shared_memory_id;
char *msg;
int user_pid;

void on_interrupt(int sig);

void on_message_received(int sig);

void screen_quit(void)
{
    endwin();
}


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define ESC 27
#define ENTER 10



int main(int argc, char *argv[])
{
     // Иницаиализация терминала
     initscr();
     keypad(stdscr, TRUE);
     // Callback на выход (завершение)
     atexit(screen_quit);
     // Разрешить прокрутку терминала
     scrollok(stdscr, TRUE);

     user_pid = getpid();

     // Обработка закрытия через Ctrl-C
     signal(SIGINT, on_interrupt);

     // Обработка закрытия через Ctrl-Z
     signal(SIGTSTP, on_interrupt);

     // Аварийное завершение
     signal(SIGABRT, on_interrupt);

     // Прием сообщения
     signal(SIGUSR1, on_message_received);
     
     // Создание массива семафоров
     semid = semget(2018, MAX_USERS, IPC_CREAT | 0666);
     // Подключение к разделяемой памяти с пользователями
     users_shared_memory_id = users_shared_memory_getter(semid);

     if (users_shared_memory_id == -1 || (users = (int *) shmat(users_shared_memory_id, 0, 0)) == NULL)
     {
          printw("couldn't get shared memory\n");
          refresh();
          exit(1);
     }

     // Подключение к разделяемой памяти с сообщениями
     msg_shared_memory_id = msg_shared_memory_getter();
     if (msg_shared_memory_id == -1 || (msg = (char *)shmat(msg_shared_memory_id, 0, 0)) == NULL)
     {
          printw("couldn't get shared memory\n");
          refresh();
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
          printw("chatroom is full\n");
          refresh();
          exit(0);
     }
     
     char new_msg[MAX_MSGLEN];
     size_t len = 0;
     ssize_t line_size;
     int key;
     int cur_msg_len = 0;

     while (1)
     {
          // Чтение символа
          key = getch();
          // Если был нажат ENTER, то фиксируем накопленное сообщение и сбрасываем его
          if (key==ENTER)
          {
               if (cur_msg_len >= MAX_MSGLEN)
               {
                    printw("\n!! MESSAGE IS TOO LONG !!\n\n");
                    cur_msg_len = 0;
                    new_msg[0] = '\0';
                    refresh();
                    continue;
               }
               new_msg[cur_msg_len]='\0';
               printw(new_msg);
               printw("\n");
               refresh();

               if (cur_msg_len > 0)
               {
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
               cur_msg_len = 0;
          }
          // Если был нажат BACKSPACE, то освобождаем последнюю позицию
          else if (key==KEY_BACKSPACE)
          {
               cur_msg_len = MAX(cur_msg_len-1, 0);
               if (cur_msg_len < MAX_MSGLEN)
               {
                    new_msg[cur_msg_len] = '\0';
               }
               // Удаляем последний введенный символ прямо из терминала
               wdelch(stdscr);
          }
          // Иначе добавляем введенный символ
          else if (isprint(key))
          {
               if (cur_msg_len < MAX_MSGLEN - 1)
               {
                    new_msg[cur_msg_len] = key;
               }
               cur_msg_len++;
               if (cur_msg_len < MAX_MSGLEN)
               {
                    new_msg[cur_msg_len] = '\0';
               }
          }
          else if (key==ESC)
          {
               kill(user_pid, SIGINT);
          }
          refresh();
     }
     
}



void on_interrupt(int sig)
{
     int close = user_exit(users_shared_memory_id, users, this_user_index, msg_shared_memory_id, msg);
     if (close == CLOSE_SHARED_MEMORY_SUCCESS)
     {
          semctl(semid, IPC_RMID, 0);
          printw("!! close shared memory !!\n");
          refresh();
     }
     else if (close == CLOSE_SHARED_MEMORY_ERROR)
     {
          printw("could not close shared memory\n");
          refresh();
     }
     printw("bye!\n");
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

     printw(msg);
     printw("\n");
     refresh();
     signal(sig, on_message_received);
}