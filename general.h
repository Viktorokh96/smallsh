#ifndef GENERAL_H
#define GENERAL_H
	#include <unistd.h>
	#include "./services/list.h"

	#define PATHSIZE	256

	/* Текущая директроия */
	char curr_path[PATHSIZE];

	/* Список аргументов в команде */ 
	list_id arg_list;

	/* Инициализация глобальных структур данных */
	int init_general();

#endif