#include "list.h"

#define list_head(hid)	(list_heads[hid])

list list_heads[MAXLISTS];		/* Массив очередей */
unsigned bit_map = 0;			/* Битовая карта свободных списков */

 void *list_entry (list *lp) /* Возвращает указатель на содержимое */
{
	if (lp->msize != 0) return (lp + 1);
	else return NULL;
}

 unsigned list_count (list_id lid)
{
	unsigned count = 0;
	list *tmp;
	list_for_each(tmp,get_head(lid)) count++;
	return count;
}

 void *list_get (unsigned num,list_id lid)
{
	if (lid < 0 || lid > MAXLISTS) return NULL;

	if (num < 0) return NULL;
	list *lp = get_head(lid);
	while(1) {
		lp = lp->mnext;
		if (num--) continue; 
		return (lp + 1);
	}
	return NULL;
}

 list *list_get_header (unsigned num,list_id lid)
{
	return (list *) list_get(num,lid)-1;
}

/* Извлечь элемент из начала списка, с последующим удалением элемента */
void *list_pop (list_id lid)
{
	if (lid < 0 || lid > MAXLISTS) return NULL;
	if (bit_seted(bit_map, 1 << lid)) {			/* Если очеред инициализирована */
		if(!list_empty(&list_heads[lid])) {
			list *lp = get_head(lid)->mnext;
			void *contain = malloc(sizeof(char)*lp->msize);
			memcpy(contain,lp+1,lp->msize);
			get_head(lid)->mnext = lp->mnext;
			free(lp);
			return contain;
		} else	return NULL;
	} else return NULL;
}

/* Проверка присутствия элемента в списке */
int list_include(list *lp, list_id lid)
{
	if (lp == NULL) return 0;	
	if (lid < 0 || lid > MAXLISTS) return 0;
	if (bit_seted(bit_map, 1 << lid)) {			/* Если очеред инициализирована */
		list *tmp;
		list_for_each(tmp,get_head(lid)) {
			if (tmp == lp) return 1;			/* Если нашли */
		}
	}

	return 0;
}

/* Удалить конкретный элемент */
void list_del_elem(list *lp, list_id lid)
{
	if (lp == NULL) return;
	if (lid < 0 || lid > MAXLISTS) return;
	if (bit_seted(bit_map, 1 << lid)) {			/* Если очеред инициализирована */
		if(!list_include(lp,lid)) return;		/* Защита от зацикливания */
		list *tmp1,*tmp2 = get_head(lid);
		/* Ищем элемент */
		for(tmp1 = tmp2; (tmp2 = tmp2->mnext) != lp; tmp1 = tmp1->mnext);
		if(tmp2 != get_head(lid)) {				/* Если нашли */
			tmp1->mnext = tmp2->mnext;			/* Связываем соседей */
			free(tmp2);							/* Уничтожаем элемент */
		} else return;
	}
}

/* Инициализирует список */
list_id init_list()
{
	list_id new_list;
	search_free(bit_map,new_list);
	if (new_list == MAXLISTS) 
		return UNINIT;
	/* Инициализация головы списка */
	list_heads[new_list].mnext = &list_heads[new_list];
	list_heads[new_list].msize = 0;
	set_bit(bit_map, 1 << new_list);		/* Обозначение занятого списка */
	return new_list;
}

void list_del (list_id lid)
{
	if (lid < 0 || lid > MAXLISTS) return;

	if (bit_seted(bit_map, 1 << lid)) {			/* Если очеред инициализирована */ 
		if(!list_empty(&list_heads[lid])) {
			list *tmp1 , *tmp2 = NULL;
			list_for_each(tmp1,get_head(lid)) { 
				free(tmp2);
				tmp2 = tmp1;
			}
			free(tmp2);						/* Удаление последнего элемента */
		}
		unset_bit(bit_map, 1 << lid);
	}
}
/* Добавить элемент в начало списка */
list *list_add (void *cont, size_t size, list_id lid)
{
	if (lid < 0 || lid > MAXLISTS) return NULL;
	
	if (bit_seted(bit_map, 1 << lid)) {			/* Если очеред инициализирована */
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
list *list_add_tail(void *cont, size_t size, list_id lid)
{
	if (lid < 0 || lid > MAXLISTS) return NULL;
	
	if (bit_seted(bit_map, 1 << lid)) {			/* Если очеред инициализирована */
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

/* Связыание дву элементов списка между собой с удаление промежуточных звеньев */
void list_connect(unsigned num1, unsigned num2, list_id lid)
{
	if (lid < 0 || lid > MAXLISTS) return;
	if (num1 < 0) return;
	if (num2 < 0) return;
	if (num1 >= num2) return;					/* У меня всё строго */

	if (bit_seted(bit_map, 1 << lid)) {			/* Если очеред инициализирована */
		list *tmp1, *tmp2, *pnext;
		tmp1 = list_get_header(num1,lid);
		tmp2 = list_get_header(num2,lid);
		pnext = tmp1->mnext;
		list_get_header(num1,lid)->mnext = list_get_header(num2,lid);;
		while((tmp1 = pnext) != tmp2) {
			pnext = tmp1->mnext;
			free(tmp1); 
		}
	}
}


 list *get_head(list_id lid) 
{
	if (lid < 0 || lid > MAXLISTS) return NULL;

	return &list_head(lid);
}