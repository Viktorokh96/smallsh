#ifndef LIST_H
#define LIST_H 1
#include <malloc.h>
#include <string.h>
#include "bits.h"			/* Для работы с битами */

typedef struct  list_head { /* Заголовок элемента */
	struct list_head *mnext;
	unsigned msize;			/* Размер содержимого */
} list;

/* Индетификатор однонаправленного циклического списка */ 
typedef unsigned int list_id;

/* Максимальное количество одновременно созданных списков */
#define MAXLISTS	32 

/* Поиск свободного списка */
#define search_free(m,p) \
	int i;	 			 \
	for (p = i = 0; bit_seted(m,1 << i) && (i < MAXLISTS);	p = ++i);

/* Прогонка по всему списку */
#define list_for_each(p,h) \
	for ((p) = (list *) (h); ((p) = (list *)(p)->mnext) != (list *)(h);) 

/* Проверка на пустоту списка */
#define list_empty(h) ((list *)(h)->mnext == (h))

/* Взятие содержимого списка */
inline void *list_entry (list *lp); /* Возвращает указатель на содержимое */

/* Подсчёт элементов списка */
inline unsigned list_count (list_id lid);

/* Получить адрес элемента списка */
inline void *list_get (unsigned num,list_id lid);

/* Получить адрес головы списка */
inline list *get_head(list_id lid);

/* Инициализирует список */
inline list_id init_list();

/* Удалить список */
inline void list_del (list_id lid);

/* Добавить элемент в начало списка */
inline list *list_add (void *cont, size_t size, list_id lid);

/* Добавить элемент в конец списка */
inline list *list_add_tail (void *cont, size_t size, list_id lid);


#endif