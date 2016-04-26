#ifndef HANDLER_H
#define HANDLER_H
#include "../general.h"
#include <unistd.h>
#include <stdlib.h>
#include "../shell.h"
#include "./jobs.h"
#include "../parses/parse.h"

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

	char *past = NULL;

	if (argv != NULL) {
		if (argv[1] != NULL) {
			if (!compare_str(argv[1],"~")) {
				if(chdir(home_path) != 0) { 
					printf("Такого каталога нет!\n");
					return 1;
				}
			}
			else if (!compare_str(argv[1],"-b")) {
				past = (char *) list_pop(path_list);
				if(!list_empty(get_head(path_list))) chdir(past);
			} 
			else { 
				list_add(curr_path,strlen(curr_path)+1,path_list);
				if(chdir(argv[1]) != 0) { 
					printf("Такого каталога нет!\n");
					return 1;
				}
			}
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

int kill_handl(void *prm)
{
	char **argv = (char **) prm;

	/* Выполнение внешней функции */
	sing_exec *ex = (sing_exec *) malloc(sizeof(sing_exec));
	ex->name = strdup(argv[0]);
	ex->mode = 0;
	ex->file = NULL;
	ex->argv = argv;
	ex->next = NULL;
	
	exec(ex);

	return 0;
}

int version_handl(void *prm)
{
	printf("Interpreter %s, version 0.001.\n",shell_name);

	return 0;
}

int declare_handl(void *prm)
{
	int i;
#ifdef __USE_GNU
	for(i = 0; environ[i]; i++) printf("%s\n", environ[i]);
#else
	for(i = 0; __environ[i] ; i++) printf("%s\n", __environ[i]);
#endif

	return 0;
}

int meow_handl(void *prm)
{
printf("../\\„„./\\.\n");
printf(".(='•'= ) .\n");
printf(".(\") „. (\").\n");
printf(". \\,\\„„/,/\n");
printf(". │„„. „│\n");
printf(". /„/„ \\„\\\n");
printf(".(„)''''(„)\n");
printf(". .. ((...\n");
printf(". . . ))..\n");
printf(". . .((..\n");

return 0;
}


#endif
