#ifndef SHMUTILS_H
#define SHMUTILS_H

// Права доступа
#define PERMS	0666

/*
* ===============================================
 *
* ===============================================
*/

/*
* ===============================================
*   ** СПИСОК ПОЛЬЗОВАТЕЛЕЙ **
 *    
 *  Имеем разделяемый сегмент памяти users
 *  Нулевой элемент - текущее количество пользователей
 *  Далее - последовательность идентификаторов процессов,
 *  обозначающая активность пользователей.
 *  Пользователи нумеруются с единицы, тогда users[i] - pid
 *  пользователя с номером i. Изначально все значения равны -1
 *
 *  При присоединении нового пользователя ищется первая свободная ячейка,
 *  пользователю присваивается соответствующий номер. При отключении
 *  пользователя соответствующая ячейка становится равной -1
 *
 *  При отправке сообщения посылается сигнал всем процессам,
 *  соответствующим активным ячейкам
 *
 *  Максимальное число пользователей - 256
 *   
* ===============================================
*/

#define USERS_SHARED_MEMORY_KEY 3003
#define MAX_USERS 256

/*
* ===============================================
 *   ** СОЗДАНИЕ РАЗДЕЛЯЕМОЙ ПАМЯТИ ПОЛЬЗОВАТЕЛЕЙ **
 *       
 *  В начале пытаемся получить доступ
 *  к разделяемой памяти, не создавая её.
 * 
 *  1) Подключение не удалось
 *      В этом случае сегмент разделяемой памяти ещё не был создан
 *      Создаем её и получаем к ней доступ,
 *      После чего задаем начальные
 *      значение: количество пользователей (нулевая
 *      ячейка)  - 0, активность (остальные ячейки) - -1
 * 
 *  2) Доступ получен
 *      Это означает, что разделяемая память уже была создана.
 *      Возвращаем её id
 * 
 *  При успешном получении памяти возвращается id
 *  Иначе: -1
 *   
* ===============================================
*/
int users_shared_memory_getter();


/*
* ===============================================
 *  ** СООБЩЕНИЕ **
 * 
 *  В отдельном сегменте разделяемой памяти
 *  будем хранить последнее актуальное сообщение
 *  
 *  Каждый пользователь будет иметь доступ
 *  к этому сегменту и сможет читать его содержимое
 * 
* ===============================================
*/
#define MSG_SHARED_MEMORY_KEY 4004
#define MAX_MSGLEN 256

/*
* ===============================================
 *  ** СОЗДАНИЕ РАЗДЕЛЯЕМОЙ ПАМЯТИ СООБЩЕНИЙ **
 * 
 *  При создании разделяемой памяти для сообщений
 *  уже не нужно следить, является ли подключение
 *  первым
 * 
 *  Подключаемся к разделяемой памяти с возможностью
 *  её создания при отсутствии и возвращаем id
 * 
* ===============================================
*/
int msg_shared_memory_getter();

/*
* ===============================================
 *   ** ОТКЛЮЧЕНИЕ ПОЛЬЗОВАТЕЛЯ **
 * 
 *   При отключении нужно уменьшить число пользователей
 *   (users[0]) на единицу
 *   Также присваиваем соответствующей ячейке
 *   (users[user_index]) значение -1
 * 
 *   Если после уменьшения количество пользователей стало
 *   равным 0, то все пользователи вышли
 *   В этом случае нужно освободить разделяемую память
 * 
* ===============================================
*/
int user_exit(int users_shared_memory_id, int *users, int user_index, int msg_shared_memory_id, char *msg);
// Возвращаемые значения
#define DETACH_SUCCESS 1
#define CLOSE_SHARED_MEMORY_SUCCESS 2
#define CLOSE_SHARED_MEMORY_ERROR 3

#endif