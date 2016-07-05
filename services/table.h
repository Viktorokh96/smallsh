#ifndef TABLE_H
#define TABLE_H

#define	ETRANGE		-1		/* Ошибка: выход за пределы границ */
#define	ETINVAL		-2		/* Ошибка: передача неверных параметров */
#define	ETPNULL		-3		/* Ошибка: передача неинициализированного указателя */
#define ETSYST		-4		/* Ошибка: непоправимая системная ошибка */

typedef struct table {
	void **pointers;		/* Массив указателей на void */
	signed int elem_quant;	/* Количество занятых элементов в массиве pointers */
	unsigned size;			/* Размер массива pointers */
	unsigned step;			/* Шаг увеличения размера массива */
} addr_table;

/* Инициализация таблицы */
int init_table(addr_table *t, unsigned step);

/* Вставка нового элемента */
int table_add(void *cont,addr_table *t);

/* Удаление элемента из таблицы адресов */
int table_del(signed num, addr_table *t);

/* Получение адреса из таблицы адресов */
void *table_get(signed num, addr_table *t);

/* Установка нового значения, существующему элементу в таблице */
int table_set(signed num, void *cont ,addr_table *t);

/* Удаление таблицы */
int free_table(addr_table *t);

/* Уничтожение таблицы вместе с содержимым */
int destroy_table(addr_table *t);

#endif