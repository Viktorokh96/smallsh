#ifndef HANDLER_H
#define HANDLER_H
#include "../general.h"
#include <unistd.h>
#include <stdlib.h>
#include "../shell.h"
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
			if (!compare_str(argv[1],"back")) {
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

int version_handl(void *prm)
{
	printf("Interpreter %s, version 0.001.\n",shell_name);

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
