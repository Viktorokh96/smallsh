#ifndef GENERAL_H
#define GENERAL_H
	#include <unistd.h>
	#include <stdlib.h>
	#include "./services/list.h"
	#include "./defines.h"
	#include "./jobs/jobs.h"
	#include <sys/types.h>
	#include <linux/limits.h>
	#include <termios.h>

#ifdef	__USE_GNU
	#define _GETWD(p)	get_current_dir_name (void);
#elif (defined __USE_XOPEN_EXTENDED && !defined __USE_XOPEN2K8) \
    || defined __USE_BSD
	#define _GETWD(p)	getwd ((p));
#else
    #define _GETWD(p) 	getcwd ((p),PATH_MAX);
#endif

	#define PATHSIZE	64

	/* Текущая директория */
	char *curr_path;

	/* Путь домашней дериктории */
	char *home_path;

	/* Имя пользователя */
	char *user_name;

	/* Полное название текущего задания */
	char *current_cmd;

	/* Предыдущая директория */
	list_id path_list;

	/* Список аргументов в команде */ 
	list_id arg_list;

	/* Индетификартор процесса оболочки */
	pid_t shell_pgid;

	/* Название оболочки */
	char *shell_name;

    struct termios shell_tmodes;

	int sh_terminal;
    
    int sh_is_interactive;

	/* Получение текущей директории */
	char *get_curr_path(char* path);

	/* Инициализация глобальных структур данных */
	int init_general();

	void del_general();
	
#endif