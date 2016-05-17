#ifndef TABLE_H
#define TABLE_H

#define NO_FREE_CONT	0
#define FREE_CONT		1

typedef struct table {
	void **pointers;	/* Массив указателей на void */
	unsigned size;		/* Размер массива pointers */
	unsigned step;		/* Шаг увеличения размера массива */
	unsigned elem_quant;	/* Количество занятых элементов в массиве pointers */
} addr_table;

/* Инициализация таблицы */
void init_table(addr_table * t, unsigned step);

/* Вставка нового элемента */
void table_add(void *cont, addr_table * t);

/* Удаление элемента из таблицы адресов */
void table_del(unsigned num, addr_table * t);

/* Получение адреса из таблицы адресов */
void *table_get(unsigned num, addr_table * t);

/* Установка нового значения, существующему элементу в таблице */
void table_set(unsigned num, void *cont, addr_table * t);

/* Удаление таблицы */
void del_table(addr_table * t, int free_mode);

#endif
