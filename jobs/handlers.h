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
			//if(tsk->status == TSK_STOPPED)			/* Если процесс спит - будим */
			current = *tsk;
				kill(tsk->pid, SIGCONT);
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
	struct queue kill_q;
	char **argv = (char **) prm;

	init_queue(&kill_q);

	for (i = 0; argv[i] != NULL; i++)
		if (*argv[i] == '%') {
			num = atoi(argv[i]+1);
			add_to_queue(num,&kill_q);
		}

	if(!queue_empty(kill_q)) {
		while((num = get_from_queue(&kill_q)) != EMPTY_Q) {
			if(num > 0 && num <= list_count(bg_jobs)) 
				tsk = (sing_exec *) list_get(num-1,bg_jobs);
			else {
				printf("Недопустимый номер! %d\n", num );
				return 1;
			}
			if (tsk->status == TSK_STOPPED)
				kill(tsk->pid,SIGCONT);
			kill(tsk->pid,SIGINT);
			printf("Killed -> %d 	%s\n",
				tsk->pid, tsk->name);
			tsk->status = TSK_KILLED;
		}
	} else {
		/* Выполнение внешней функции */
		sing_exec *ex = (sing_exec *) malloc(sizeof(sing_exec));
		ex->name = strdup(argv[0]);
		ex->mode = 0;
		ex->file = NULL;
		ex->argv = argv;
		ex->next = NULL;
		
		exec(ex);		

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
					printf("Killed -> %d 	%s\n",
						tsk->pid, tsk->name);
					tsk->status = TSK_KILLED;
				}
			}
		}
	}

	update_jobs();

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
