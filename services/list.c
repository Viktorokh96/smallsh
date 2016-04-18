#include "list.h"

#define list_head(hid)	(list_heads[hid])

list list_heads[MAXLISTS];		/* Массив очередей */
unsigned bit_map = 0;			/* Битовая карта свободных списков */

inline void *list_entry (list *lp) /* Возвращает указатель на содержимое */
{
	if (lp->msize != 0) return (lp + 1);
	else return NULL;
}

inline unsigned list_count (list_id lid)
{
	unsigned count = 0;
	list *tmp;
	list_for_each(tmp,get_head(lid)) count++;
	return count;
}

inline void *list_get (unsigned num,list_id lid)
{
	if (lid < 0 || lid > MAXLISTS) return NULL;

	list *lp;
	if ((num > list_count(lid)-1) || (num < 0)) return NULL;
	list_for_each(lp,get_head(lid)) {
		if (num--) continue; 
		if (lp->msize != 0) return (lp + 1);
	}
	return NULL;
}

/* Инициализирует список */
inline list_id init_list()
{
	list_id new_list;
	search_free(bit_map,new_list);
	if (new_list == MAXLISTS) 
		return -1;
	/* Инициализация головы списка */
	list_heads[new_list].mnext = &list_heads[new_list];
	list_heads[new_list].msize = 0;
	set_bit(bit_map, 1 << new_list);		/* Обозначение занятого списка */
	return new_list;
}

inline void list_del (list_id lid)
{
	if (lid < 0 || lid > MAXLISTS) return;

	if (bit_seted(bit_map, lid)) {			/* Если очеред инициализирована */ 
		if(!list_empty(&list_heads[lid])) {
			list *tmp1 , *tmp2 = NULL;
			list_for_each(tmp1,get_head(lid)) { 
				free(tmp2);
				tmp2 = tmp1;
			}
			free(tmp2);						/* Удаление последнего элемента */
			unset_bit(bit_map, 1 << lid);
		}
	}
}
/* Добавить элемент в начало списка */
inline list *list_add (void *cont, size_t size, list_id lid)
{
	if (lid < 0 || lid > MAXLISTS) return NULL;
	
	if (bit_seted(bit_map, lid)) {			/* Если очеред инициализирована */
		list *new_list;
		/* Выделяем память под элемент списка и содержимое */
		if(!(new_list = malloc(sizeof(list) + size))) return NULL;
		new_list->msize = size;
		new_list->mnext = list_heads[lid].mnext;
		list_heads[lid].mnext = new_list;
		memcpy(new_list + 1,cont,size); 

		return new_list;						/* На случай, если понадобиться */
	} 
	else return NULL;
}

/* Добавить элемент в конец списка */
inline list *list_add_tail(void *cont, size_t size, list_id lid)
{
	if (lid < 0 || lid > MAXLISTS) return NULL;
	
	if (bit_seted(bit_map, lid)) {			/* Если очеред инициализирована */
		list *new_list, *q;
		/* Выделяем память под элемент списка и содержимое */
		if(!(new_list = malloc(sizeof(list) + size))) return NULL;
		new_list->msize = size;

		for (q = &list_heads[lid]; q->mnext != &list_heads[lid]; q = q->mnext);
		new_list->mnext = &list_heads[lid];
		q->mnext = new_list;
		memcpy(new_list + 1,cont,size); 

		return new_list;						/* На случай, если понадобиться */
	} 
	else return NULL;
}


inline list *get_head(list_id lid) 
{
	if (lid < 0 || lid > MAXLISTS) return NULL;

	return &list_head(lid);
}