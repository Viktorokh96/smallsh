#ifndef HANDLER_H
#define HANDLER_H
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
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
	/* prints current working directory */
	printf("%s\n", getenv("PWD"));
	return 0;
}

/* Transition on directories */
int cd_handl(void *prm)
{
	char **argv = (((sing_exec *) prm) -> argv);

	char *past = NULL;

	if (argv != NULL) {
		if (argv[1] != NULL) {
			if (!strcmp(argv[1],"-b")) {
				if (past_path.elem_quant != 0) {
					past = (char *) table_get(past_path.elem_quant-1,&past_path);
					table_del(past_path.elem_quant-1,&past_path);
					if(past != NULL) {
						chdir(past);
						free(past);
					}	
				}
			} 
			else { 
				table_add(_STR_DUP(curr_path),&past_path);
				if(chdir(full_path(argv[1])) != 0) { 
					printf("Такого каталога нет!\n");
					return 1;
				}
			}
		} else {
			table_add(_STR_DUP(curr_path),&past_path);
			if(chdir(full_path(home_path)) != 0) { 
					printf("Такого каталога нет!\n");
					return 1;
				}
		}
	curr_path = get_curr_path(curr_path);
	/* Change  value of PWD, that will be usefull for other applications */
		setenv("PWD",curr_path,1);	
	}

	return 0;
}

int jobs_handl(void *prm)
{
	int i;
	task *tsk;
	int show_pid = 0;
	char **argv = (((sing_exec *) prm) -> argv);

	update_jobs();

	for(i = 0; argv[i] != NULL; i++) 
		if(!strcmp(argv[i],"-i")) show_pid = 1;
	
	for(i = 0; i < bg_jobs.elem_quant; i++) {
		tsk = (task *) table_get(i,&bg_jobs);
		printf("[%d]%c %s\t\t%s",i+1,(i+1 == 1) ? '+' : '-', 
			(tsk->status == TSK_RUNNING)? 
			"Running" : "Stopped",tsk->name);
		if(show_pid) printf("\t\tPID: %d", tsk->pgid);
		printf("\n");
	}

	return 0;
}

/* Command for transfer process from the background mode to current */
int fg_handl(void *prm)
{
	int num;
	int stat;
	task *tsk = NULL;
	char **argv = (((sing_exec *) prm) -> argv);

	update_jobs();

	if (argv[1] != NULL && *argv[1] == '%') {	/* If user entered number of process */
		num = atoi(argv[1]+1);
		if(num > 0 && num <= bg_jobs.elem_quant) {
			tsk = (task *) table_get(num-1,&bg_jobs);
			table_del(num-1,&bg_jobs);
		} else {
			printf("Такой задачи нет %d\n", num );
			return 1;
		}
	} else if(bg_jobs.elem_quant != 0) {
		/* If number isn't specified - we get the first */
		tsk = (task *) table_get(0,&bg_jobs);
		table_del(0,&bg_jobs);
	}

	/* tsk - it's the copy of task in bg_jobs */
	if(tsk != NULL) {
		if(tsk->status == TSK_STOPPED)			/* If process sleeps - awake */
			kill(-(tsk->pgid), SIGCONT);
		tsk->mode = RUN_ACTIVE;					/* Switching to active mode */
		tsk->status = TSK_RUNNING;
		printf("%s\n",tsk->name);
		set_task_to_term(tsk);					/* bind group of process to the terminal */
		stat = wait_child(tsk->current_ex);
		exec_next(tsk->current_ex,stat);
	}

	return 0;
}

/* Command for transfer process from the sleep mode to running in background */
int bg_handl(void *prm)
{
	int num;
	task *tsk = NULL;
	char **argv = (((sing_exec *) prm) -> argv);

	update_jobs();

	if (argv[1] != NULL && *argv[1] == '%') {	/* If user entered number of process */
		num = atoi(argv[1]+1);
		if(num > 0 && num <= bg_jobs.elem_quant) {
			tsk = (task *) table_get(num-1,&bg_jobs);
		} else {
			printf("Такой задачи нет %d\n", num );
			return 1;
		}
	} else if(bg_jobs.elem_quant != 0)
		/* If number isn't specified - we get the first */
		tsk = (task *) table_get(0,&bg_jobs);

	/* tsk - it's a original (!) of task in bg_jobs */
	if(tsk != NULL) {
		if(tsk->status == TSK_STOPPED)			/* If process sleeps - awake */
			kill(-(tsk->pgid), SIGCONT);
		tsk->status = TSK_RUNNING;
	}

	return 0;
}

/* Overloading kill, for more functionality */	
int kill_handl(void *prm)
{
	task *tsk;
	int i, num = 0;
	char pidbuff[10];
	sing_exec *ex = (sing_exec *) prm;

	for (i = 0; ex->argv[i] != NULL; i++)
		if (*(ex->argv)[i] == '%') {		/* If user entered number of process */
			num = atoi(ex->argv[i]+1);
			if(num > 0 && num <= bg_jobs.elem_quant)
				tsk = (task *) table_get(num-1,&bg_jobs);
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

	/* Call of external kill */
	ex->handler = NULL;
	exec_cmd(ex,NO_NEXT);

	return 0;
}

int version_handl(void *prm)
{
	printf("Minimalistic interpreter %s, version 0.005\tWelcome!\n",shell_name);

	return 0;
}

int declare_handl(void *prm)
{
	int i;

	for(i = 0; _ENVIRON[i] ; i++) printf("%s\n", _ENVIRON[i]);

	return 0;
}

/* Overloading ls, while aliases aren't implemented  */	
int ls_handl(void *prm)
{
	int i;
	sing_exec *ex = (sing_exec *) prm;

	for(i = 0; ex->argv[i] != NULL; i++);
	ex->argv = realloc(ex->argv,(i+2)*sizeof(char *));

	ex->argv[i] = _STR_DUP("--color=auto");
	ex->argv[i+1] = NULL;

	/* Call of external ls */
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
