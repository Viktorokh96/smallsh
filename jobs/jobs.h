#ifndef JOBS_H
#define JOBS_H 
	
	#include "../services/list.h"
	#include <unistd.h>

	typedef struct job_st {
		char *name;					/* Имя команды */
		int (*handler)(void *);  	/* Обработчик */
	} job;

	typedef struct task_sh {
		char *name;
		pid_t pid;
		pid_t ppid;
	} task_sh;

	job job_sh; 		/* Структура для описания обработчика встроенной функции */
	task_sh tsk_sh;	 	/* Структура для описания процесса */

	/* Список выполняемых программ оболочки */
	list_id sh_jobs;

	/* Список программ выполняющихся в фоновом режиме */
	list_id bg_jobs;

	void init_jobs();
	
	void del_jobs();

	int (* is_shell_cmd(char *cmd)) (void *);

	/* Добавление обработчика встроенной функции оболочки */
	#define add_job(n,h) \
				job_sh.name = (n); \
				job_sh.handler = &(h); \
				list_add(&job_sh,sizeof(job),sh_jobs);

	#define add_bg_job(n,id,parid) \
				tsk_sh.name = (n); \
				tsk_sh.pid = (id); \
				tsk_sh.ppid = (parid);\
				list_add(&tsk_sh,sizeof(task_sh),bg_jobs);

#endif