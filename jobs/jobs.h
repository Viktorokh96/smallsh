#ifndef JOBS_H
#define JOBS_H 
	
	#include "../services/list.h"
	#include <unistd.h>

	typedef struct job_st {
		char *name;					/* Имя команды */
		int (*handler)(void *);  	/* Обработчик */
	} job;

	/* Список выполняемых программ оболочки */
	list_id sh_jobs;

	void init_jobs();
	
	void del_jobs();

	int (* is_shell_cmd(char *cmd)) (void *);

#endif