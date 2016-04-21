#ifndef HANDLER_H
#define HANDLER_H
#include "../general.h"
#include <unistd.h>
#include <stdlib.h>
#include "../shell.h"

int exit_handl(void *prm)
{
	end_of_work();
	return 0;
}

int pwd_handl(void *prm)
{
	printf("%s\n", getenv("PWD"));
	return 0;
}

int cd_handl(void *prm)
{
	char **argv = (char **) prm;

	if (argv != NULL) {
		if(chdir(argv[1]) != 0) { 
			printf("Такого каталога нет!\n");
			return 1;
		}
#ifdef	__USE_GNU
		curr_path = get_current_dir_name (void);
#endif
#if (defined __USE_XOPEN_EXTENDED && !defined __USE_XOPEN2K8) \
    || defined __USE_BSD
		getwd (curr_path);
#else
		getcwd (curr_path,PATHSIZE);
#endif
	/* Установка нового значения в переменную окружения PWD */
		setenv("PWD",curr_path,1);	
	}

	return 0;
}

int version_handl(void *prm)
{
	printf("Interpreter %s, version 0.001.\n",shell_name);

	return 0;
}


#endif
