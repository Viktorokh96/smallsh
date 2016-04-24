#ifndef GENERAL_H
#define GENERAL_H
	#include <unistd.h>
	#include <stdlib.h>
	#include "./services/list.h"
	#include "./defines.h"
	#include <sys/types.h>
	#include <pwd.h> 	

	#define PATHSIZE	256

	/* Текущая директория */
	char *curr_path;

	/* Путь домашней дериктории */
	char *home_path;

	/* Имя пользователя */
	char *user_name;

	/* Предыдущая директория */
	list_id path_list;

	/* Список аргументов в команде */ 
	list_id arg_list;

	/* Индетификартор процесса оболочки */
	pid_t shell_pid;

	/* Название оболочки */
	char *shell_name;

	/* Инициализация глобальных структур данных */
	int init_general();

	void del_general();
#endif