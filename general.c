#include <malloc.h>
#include <errno.h>
#include <pwd.h> 	
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "general.h"
#include "./jobs/jobs.h"

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
	sh_terminal = open("/dev/tty", O_ASYNC | O_RDWR);
    sh_is_interactive = isatty(sh_terminal);

	
    if (sh_is_interactive)
    {
            while (tcgetpgrp (sh_terminal) !=    
                    (shell_pgid = GET_PGID(0)))
            kill (-shell_pgid, SIGTTIN);
            tcsetpgrp (sh_terminal, shell_pgid);
            tcgetattr(sh_terminal,&shell_tmodes);
    }
    
	init_table(&past_path,5);

	curr_path = get_curr_path(curr_path);

	if(curr_path != NULL)
		table_add(_STR_DUP(curr_path),&past_path);

	home_path = getenv("HOME");
	user_name = getpwuid(geteuid())->pw_name;				/* Получаем имя пользователя */
	
	return 0;
}

void del_general()
{
	del_table(&past_path, FREE_CONT);
}