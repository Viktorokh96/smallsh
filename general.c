#include <malloc.h>
#include <errno.h>
#include <pwd.h> 	
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
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
	sh_terminal = STDIN_FILENO;
    sh_is_interactive = isatty(sh_terminal);

    if (sh_is_interactive)
    {
            while (tcgetpgrp (sh_terminal) !=    
                    (shell_pgid = GET_PGID(0)))
            kill (-shell_pgid, SIGTTIN);

            shell_pgid = getpid ();
            if (_SETPGID(shell_pgid,shell_pgid) < 0)
            {
                    perror ("Couldn't put the shell in its own process group");
                    _exit (1);
            }
            tcsetpgrp (sh_terminal, shell_pgid);
            tcgetattr(sh_terminal,&shell_tmodes);
    }

	path_list = init_list();

	curr_path = get_curr_path(curr_path);

	list_add(curr_path,strlen(curr_path)+1,path_list);

	home_path = getenv("HOME");
	user_name = getpwuid(geteuid())->pw_name;				/* Получаем имя пользователя */
	current.gpid = 0;
	
	return 0;
}

void del_general()
{
	list_del(&arg_list);
	list_del(&path_list);
}