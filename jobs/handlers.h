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
	sing_exec *tsk;
	int show_pid = 0;
	char **argv = (char **) prm;

	update_jobs();

	for(i = 0; argv[i] != NULL; i++) 
		if(!compare_str(argv[i],"-i")) show_pid = 1;
	
	i = 1;
	list_for_each(tmp,get_head(bg_jobs)) {
		tsk = (sing_exec*) list_entry(tmp);
		printf("[%d]%c %s\t\t%s",i,(i == 1) ? '+' : '-', 
			(tsk->status == TSK_RUNNING)? 
			"Running" : "Stopped",tsk->name);
		if(show_pid) printf("\t\tPID: %d", tsk->pid);
		printf("\n");
		i++;	
	}

	return 0;
}

int fg_handl(void *prm)
{
	list *tmp;
	sing_exec *tsk;
		
	list_for_each(tmp,get_head(bg_jobs)) {
			tsk = (sing_exec *) malloc(sizeof(sing_exec));
			memcpy(tsk,(sing_exec*) list_entry(tmp),sizeof(sing_exec));
			if(tsk->status == TSK_STOPPED)			/* Если процесс спит - будим */
				kill(tsk->pid, SIGCONT);
			current = *tsk;
			wait_child(tsk);
			list_del_elem(tmp,bg_jobs);
			free(tsk);
			return 0;
	}

	return 0;
}

/* Перегрузка готовой программы kill, для дополнительного функционала */	
int kill_handl(void *prm)
{
	list *tmp;
	sing_exec *tsk;
	pid_t pid;
	int i, num = 0;
	char pidbuff[10];
	char **argv = (char **) prm;

	for (i = 0; argv[i] != NULL; i++)
		if (*argv[i] == '%') {
			num = atoi(argv[i]+1);
			/* add_to_queue(num,&kill_q); */
			if(num > 0 && num <= list_count(bg_jobs)) 
				tsk = (sing_exec *) list_get(num-1,bg_jobs);
			else {
				printf("Такой задачи нет %d\n", num );
				return 1;
			}
			free (argv[i]);
			sprintf(pidbuff,"%d",tsk->pid);
			argv[i] = _STR_DUP(pidbuff);
			if (tsk->status == TSK_STOPPED)
				kill(tsk->pid,SIGCONT);
		}

	/* Выполнение внешней функции */
	sing_exec *ex = (sing_exec *) malloc(sizeof(sing_exec));
	ex->name = strdup(argv[0]);
	ex->mode = 0;
	ex->file = NULL;
	ex->argv = argv;
	ex->next = NULL;
	
	exec_cmd(ex);		

	i = 1;
	while (*argv[i] == '-') i++;
	for(; argv[i] != NULL; i++) {
		pid = atoi(argv[i]);
		list_for_each(tmp,get_head(bg_jobs)) {
			tsk = (sing_exec*) list_entry(tmp);
			if(tsk->pid == pid) {
				if (tsk->status == TSK_STOPPED)
					kill(tsk->pid,SIGCONT);
				waitpid(pid,NULL,WNOHANG);
				tsk->status = TSK_KILLED;
			}
		}
	}

	update_jobs();

	return 0;
}

int version_handl(void *prm)
{
	printf("Minimalistic interpreter %s, version 0.003.\n",shell_name);

	return 0;
}

int declare_handl(void *prm)
{
	int i;

	for(i = 0; __environ[i] ; i++) printf("%s\n", _ENVIRON[i]);

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
