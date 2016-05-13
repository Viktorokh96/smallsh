#include "table.h"
#include <stdlib.h>
#include <stdio.h>

#define check_table(t,ret) \
	do {	\
		if ((t) == NULL) {	\
			fprintf(stderr, \
				"table init error: invalid addres to struct addr_table\n");\
			ret;	\
		}	\
	} while(0)	

#define check_range(num,t,ret) \
	do { \
		if (num < 0 || num > (t -> elem_quant-1)) { \
			fprintf(stderr, \
				"table get error: invalid value of number argument\n"); \
			ret; \
		}	\
	} while(0)

void swap(void **p1, void **p2)
{
	void *tmp = *p1;
	*p1 = *p2;
	*p2 = tmp;
}

/* Инициализация таблицы */
void init_table(addr_table *t, unsigned step)
{
	if (step < 1) {
		fprintf(stderr, 
			"table init error: step value is less than 1\n");
		return;
	}

	check_table(t,return);

	t -> pointers = NULL;
	t -> size = 0;
	t -> step = step;
	t -> elem_quant = 0;
}

void table_realloc(addr_table *t)
{
	check_table(t,return);

	t -> size += t -> step;
	t -> pointers = realloc(t -> pointers, t -> size * sizeof(void *));
	if (t -> pointers == NULL) {
		fprintf(stderr, 
			"table realloc error: couldn't request memory \n");
	}
}

/* Вставка нового элемента */
void table_add(void *cont,addr_table *t)
{
	check_table(t,return);

	if(t -> size == t -> elem_quant) 
		table_realloc(t);
	t -> pointers[t -> elem_quant] = cont;
	t -> elem_quant++;
}

/* Удаление элемента из таблицы адресов */
void table_del(unsigned num, addr_table *t)
{
	check_table(t,return);
	check_range(num,t,return);

	t -> pointers[num] = NULL;
	t -> elem_quant--;
	while((num) < (t -> elem_quant)) {
		swap(&t->pointers[num],&t->pointers[num+1]); 
		num++;
	}
}

/* Получение адреса из таблицы адресов */
void *table_get(unsigned num, addr_table *t)
{
	check_table(t,return NULL);
	check_range(num,t,return NULL);

	return t -> pointers[num];
}

/* Установка нового значения, существующему элементу в таблице */
void table_set(unsigned num, void *cont ,addr_table *t)
{
	check_table(t,return);
	check_range(num,t,return);

	t -> pointers[num] = cont;
}

/* Удаление таблицы */
void del_table(addr_table *t, int free_mode)
{
	check_table(t,return);

	if(free_mode == FREE_CONT) {
		int i;
		for(i = 0; i < t -> elem_quant; i++) 
			if(t -> pointers[i]) free(t -> pointers[i]);
	}
	
	free(t -> pointers);
}