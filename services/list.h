#ifndef LIST_H
#define LIST_H 1
#include <malloc.h>
#include <string.h>
#include "bits.h"			/* Для работы с битами */
#include "../defines.h"

#define EMPTY_Q		(-1)

typedef struct  list_head { /* Заголовок элемента */
	struct list_head *mnext;
	unsigned msize;			/* Размер содержимого */
} list;

struct queue {	/* Очередь специальных символов (в виде колцевого буфера) */
	int queue[Q_LENG];
	int prod;			/* Указатель на новую позицию для добавления в очередь */
	int cons;			/* Указатель на позицию для следующего чтения из очереди */
	int (*next)(int p);	/* Метод, возвращающий следущий элемент */
};


/* Индетификатор однонаправленного циклического списка */ 
typedef unsigned int list_id;

/* Максимальное количество одновременно созданных списков */
#define MAXLISTS	32

/* Значение неинициализированного списка */
#define UNINIT	-1 

/* Поиск свободного списка */
#define search_free(m,p) \
	int i;	 			 \
	for (p = i = 0; bit_seted(m,1 << i) && (i < MAXLISTS);	++i , p = i);

/* Прогонка по всему списку */
#define list_for_each(p,h) \
	for ((p) = (list *) (h); ((p) = (list *)(p)->mnext) != (list *)(h);) 

/* Проверка на пустоту списка */
#define list_empty(h) ((list *)(h)->mnext == (h))

/* Взятие содержимого списка */
 void *list_entry (list *lp); /* Возвращает указатель на содержимое */

/* Подсчёт элементов списка */
 unsigned list_count (list_id lid);

/* Получить адрес элемента списка */
 void *list_get (unsigned num,list_id lid);

/* Получить адрес головы списка */
 list *get_head(list_id lid);

/* Получить адрес заголовка */
 list *list_get_header (unsigned num,list_id lid);

/* Извлечь элемент из начала списка, с последующим удалением элемента */
	void *list_pop (list_id lid);

/* Инициализирует список */
list_id init_list();

/* Удалить список */
void list_del (list_id lid);

/* Связыание дву элементов списка между собой с удаление промежуточных звеньев */
void list_connect(unsigned num1, unsigned num2, list_id lid);

/* Добавить элемент в начало списка */
list *list_add (void *cont, size_t size, list_id lid);

/* Добавить элемент в конец списка */
list *list_add_tail (void *cont, size_t size, list_id lid);

/* Удалить конкретный элемент */
void list_del_elem(list *lp, list_id lid);

/* Реализация очереди */
void init_queue(struct queue *q);

int queue_next (int p);

int queue_empty(struct queue q);

/* Добавление значения в очередь */
void add_to_queue(int val, struct queue *q);

/* Взятие специального символа из очереди */
int get_from_queue(struct queue *q);

#endif