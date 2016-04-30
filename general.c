#include <malloc.h>
#include <errno.h>
#include <pwd.h> 	
#include "general.h"

char *path_alloc(unsigned char size, char* path) 
{
	if (path != NULL) free(path);				/* Уничтожаем старую строку */
	return (char *) malloc(PATHSIZE*size*sizeof(char));
}

char *get_curr_path(char* path)
{
	static unsigned char n = 1;
	
	path = path_alloc(n,path);
	path = _GETWD(path);

	while(path == NULL) {					/* Проверка на ошибки */
		switch(errno) {
			case EFAULT:
			case EINVAL: 
					path = path_alloc(n,path);
					path = _GETWD(path);
				break;
			case ERANGE: 
					path = path_alloc(++n,path);
					path = _GETWD(path);
				break;
		}
	}

	return path;
}

int init_general() 
{

	path_list = init_list();

	curr_path = get_curr_path(curr_path);

	list_add(curr_path,strlen(curr_path)+1,path_list);
	shell_pid = setsid();									/* Создание новой сессии */
	home_path = getenv("HOME");
	user_name = getpwuid(geteuid())->pw_name;				/* Получаем имя пользователя */
	current.pid = 0;
	
	return 0;
}

void del_general()
{
	list_del(arg_list);
	list_del(path_list);
}