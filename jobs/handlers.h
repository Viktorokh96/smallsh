#ifndef HANDLER_H
#define HANDLER_H
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include "../general.h"
#include "../shell.h"
#include "./jobs.h"
#include "../parses/parse.h"

int exit_handl(void *prm)
{
	end_of_work(EXIT_SUCCESS);
	return 0;
}

int pwd_handl(void *prm)
{
	printf("%s\n", getenv("PWD"));
	return 0;
}

int cd_handl(void *prm)
{
	char **argv = (((sing_exec *) prm) -> argv);

	char *past = NULL;

	if (argv != NULL) {
		if (argv[1] != NULL) {
			if (!compare_str(argv[1],"-b")) {
				past = (char *) list_pop(path_list);
				if(!list_empty(get_head(path_list))) chdir(past);
			} 
			else { 
				list_add(curr_path,strlen(curr_path)+1,path_list);
				if(chdir(full_path(argv[1])) != 0) { 
					printf("Такого каталога нет!\n");
					return 1;
				}
			}
		} else {
			if(chdir(full_path(home_path)) != 0) { 
					printf("Такого каталога нет!\n");
					return 1;
				}
		}
	curr_path = get_curr_path(curr_path);
	/* Установка нового значения в переменную окружения PWD */
		setenv("PWD",curr_path,1);	
	}

	return 0;
}

int jobs_handl(void *prm)
{
	int i;
	list *tmp;
	task *tsk;
	int show_pid = 0;
	char **argv = (((sing_exec *) prm) -> argv);

	update_jobs();

	for(i = 0; argv[i] != NULL; i++) 
		if(!compare_str(argv[i],"-i")) show_pid = 1;
	
	i = 1;
	list_for_each(tmp,get_head(bg_jobs)) {
		tsk = (task *) list_entry(tmp);
		printf("[%d]%c %s\t\t%s",i,(i == 1) ? '+' : '-', 
			(tsk->status == TSK_RUNNING)? 
			"Running" : "Stopped",tsk->name);
		if(show_pid) printf("\t\tPID: %d", tsk->pgid);
		printf("\n");
		i++;	
	}

	return 0;
}

/* Команда для перевода процесса из фонового режима в текущий */
int fg_handl(void *prm)
{
	int num;
	int stat;
	task *tsk = NULL;
	char **argv = (((sing_exec *) prm) -> argv);

	update_jobs();

	if (argv[1] != NULL && *argv[1] == '%') {	/* Если пользователь ввел номер процесса */
		num = atoi(argv[1]+1);
		if(num > 0 && num <= list_count(bg_jobs)) {
			tsk = (task *) malloc(sizeof(sing_exec));
			memcpy(tsk,(task *) list_get(num-1,bg_jobs),sizeof(task));
			list_del_elem(list_get_header(num-1,bg_jobs),bg_jobs);
		} else {
			printf("Такой задачи нет %d\n", num );
			return 1;
		}
	} else if(!list_empty(get_head(bg_jobs))) 
		tsk = (task *) list_pop(bg_jobs); /* Иначе выводим первый процесс в списке */

	/* tsk - теперь это копия того задания, что был в спискке bg_jobs */
	if(tsk != NULL) {
		if(tsk->status == TSK_STOPPED)			/* Если процесс спит - будим */
			kill(-(tsk->pgid), SIGCONT);
		tsk->mode = RUN_ACTIVE;					/* Перевод в активный режим */
		set_task_to_term(tsk);					/* Привязываем группу процесса к терминалу */
		stat = wait_child(tsk->current_ex);
		exec_next(tsk->current_ex,stat);
		free(tsk);
	}

	return 0;
}

/* Команда для перевода процесса из спящего режима в выполняемый на фоне */
int bg_handl(void *prm)
{
	int num;
	task *tsk = NULL;
	char **argv = (((sing_exec *) prm) -> argv);

	update_jobs();

	if (argv[1] != NULL && *argv[1] == '%') {	/* Если пользователь ввел номер процесса */
		num = atoi(argv[1]+1);
		if(num > 0 && num <= list_count(bg_jobs)) {
			tsk = (task *) list_get(num-1,bg_jobs);
		} else {
			printf("Такой задачи нет %d\n", num );
			return 1;
		}
	} else if(!list_empty(get_head(bg_jobs))) 
		tsk = (task *) list_get(0,bg_jobs); /* Иначе выводим первый процесс в списке */

	/* tsk - теперь это оригинал (!) того задания, что есть в спискке bg_jobs */
	if(tsk != NULL) {
		if(tsk->status == TSK_STOPPED)			/* Если процесс спит - будим */
			kill(-(tsk->pgid), SIGCONT);
		tsk->mode = RUN_BACKGR;					/* Перевод в активный режим */
		tsk->status = TSK_RUNNING;
	}

	return 0;
}

/* Перегрузка готовой программы kill, для дополнительного функционала */	
int kill_handl(void *prm)
{
	task *tsk;
	int i, num = 0;
	char pidbuff[10];
	sing_exec *ex = (sing_exec *) prm;

	for (i = 0; ex->argv[i] != NULL; i++)
		if (*(ex->argv)[i] == '%') {
			num = atoi(ex->argv[i]+1);
			if(num > 0 && num <= list_count(bg_jobs)) 
				tsk = (task *) list_get(num-1,bg_jobs);
			else {
				printf("Такой задачи нет %d\n", num );
				return 1;
			}
			free (ex->argv[i]);
			sprintf(pidbuff,"%d",tsk->pgid);
			ex->argv[i] = _STR_DUP(pidbuff);
			if (tsk->status == TSK_STOPPED)
				kill(-(tsk->pgid),SIGCONT);
		}

	/* Вызов внешней функции kill */
	ex->handler = NULL;
	exec_cmd(ex,NO_NEXT);

	return 0;
}

int version_handl(void *prm)
{
	printf("Minimalistic interpreter %s, version 0.004\tWelcome!\n",shell_name);

	return 0;
}

int declare_handl(void *prm)
{
	int i;

	for(i = 0; __environ[i] ; i++) printf("%s\n", _ENVIRON[i]);

	return 0;
}

int ls_handl(void *prm)
{
	int i;
	sing_exec *ex = (sing_exec *) prm;

	for(i = 0; ex->argv[i] != NULL; i++);
	ex->argv = realloc(ex->argv,sizeof(char)*(i+1));

	ex->argv[i] = _STR_DUP("--color=auto");
	ex->argv[i+1] = NULL;

	/* Вызов внешней функции ls */
	ex->handler = NULL;
	exec_cmd(ex,NO_NEXT);

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
