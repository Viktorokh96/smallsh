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

	typedef struct single_execute {	/* Структура исполняемой единицы */
		char *name;
		char **argv;				/* Аргументы исполняемой единицы */
		char **files;				/* Файлы в которые или из которых идёт в\в данных */
		struct
			single_execute *next;	/* Следующая исполяемая единица в цепочке */		
		int (*exec_func)(struct 
			 single_execute *self);/* Функция исполнения */
	} sing_exec;

	job job_sh; 		/* Структура для описания обработчика встроенной функции */
	task_sh tsk_sh;	 	/* Структура для описания процесса */

	/* Список выполняемых программ оболочки */
	list_id sh_jobs;

	/* Список программ выполняющихся в фоновом режиме */
	list_id bg_jobs;

	void init_jobs();
	
	void del_jobs();

	/* Добавление специального исмвола в очередь */
	void add_spec(int val);

	/* Взятие специального символа из очереди */
	int  get_spec();

	/* Определение встроенной функции */
	int (* is_shell_cmd(char *cmd)) (void *);

	/* Подготовка аргументов для команды */
	char **prepare_args(int num, unsigned mode);

	/* Содание очереди на исполнение */
	sing_exec *create_exec_queue();

	/* Освобождение памяти, занятую под исп.единицу */
	void free_exec(sing_exec *ex);

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