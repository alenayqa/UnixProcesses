#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include "message.h"
#include "shmutils.h"


void on_keyboard_interrupt(int sig);



int main(int argc, char *argv[])
{
     int _pid = getpid();
     printf("%d\n", _pid);

     // Обработка закрытия через Ctrl-C
     signal(SIGINT, on_keyboard_interrupt);
     
     // Идентификатор разделяемой памяти
     int shared_memory_id;

     while (1)
     {
          
     }
}

void on_keyboard_interrupt(int sig)
{
     // char  c;

     // signal(sig, SIG_IGN);
     // printf("Do you want to stop the server? [y/n]\n");
     // c = getchar();
     // if (c == 'y' || c == 'Y')
     //      exit(0);
     // else
     //      signal(SIGINT, on_keyboard_interrupt);
     // getchar(); // Get new line character
     printf("bye!\n");
     exit(0);
}