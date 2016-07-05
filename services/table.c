#include "table.h"
#include <stdlib.h>

#define check_table(t,ret) \
	do {	\
		if ((t) == NULL)	\
			ret;	\
	} while(0)

#define check_range(num,t,ret) \
	do { \
		if (num < 0 || num > (t -> elem_quant-1)) \
			ret; \
	} while(0)

/* Инициализация таблицы */
int init_table(addr_table *t, unsigned step)
{
	if (step < 1)
		return ETINVAL;

	check_table(t,return ETPNULL);

	t -> pointers = NULL;
	t -> size = 0;
	t -> step = step;
	t -> elem_quant = 0;

	return 0;
}

int table_realloc(addr_table *t)
{
	check_table(t,return ETPNULL);

	t -> size += t -> step;
	t -> pointers = realloc(t -> pointers, t -> size * sizeof(void *));
	if (t -> pointers == NULL)
		return ETSYST;

	return 0;
}

/* Вставка нового элемента */
int table_add(void *cont,addr_table *t)
{
	check_table(t,return ETPNULL);

	if(t -> size == t -> elem_quant) 
		if(table_realloc(t) == ETSYST)
			return ETSYST;
	t -> pointers[t -> elem_quant] = cont;
	t -> elem_quant++;

	return 0;
}

/* Удаление элемента из таблицы адресов */
int table_del(signed num, addr_table *t)
{
	check_table(t,return ETPNULL);
	check_range(num,t,return ETRANGE);

	t -> elem_quant--;
	while((num) < (t -> elem_quant)) {
		t->pointers[num] = t->pointers[num+1];
		num++;
	}

	return 0;
}

/* Получение адреса из таблицы адресов */
void *table_get(signed num, addr_table *t)
{
	check_table(t,return NULL);
	check_range(num,t,return NULL);
	
	return t -> pointers[num];
}

/* Установка нового значения, существующему элементу в таблице */
int table_set(signed num, void *cont ,addr_table *t)
{
	check_table(t,return ETPNULL);
	check_range(num,t,return ETRANGE);

	t -> pointers[num] = cont;

	return 0;
}

/* Удаление таблицы */
int free_table(addr_table *t)
{
	check_table(t,return ETPNULL);
	
	free(t -> pointers);

	return 0;
}

/* Уничтожение таблицы вместе с содержимым */
int destroy_table(addr_table *t)
{
	check_table(t,return ETPNULL);

	void *p;
	while((p = table_get(0,t)) != NULL) {
		free(p);
		table_del(0,t);
	}

	free_table(t);

	return 0;
}