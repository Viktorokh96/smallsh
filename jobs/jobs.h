#ifndef JOBS_H
#define JOBS_H 
	
	#include <sys/types.h> 	/* Спасибо stackoverflow ( для std=c99 ) */
	#include <unistd.h>
	#include "../services/list.h"
	#include "../defines.h"

	#if defined  _SVID_SOURCE || _BSD_SOURCE || _XOPEN_SOURCE >= 500 \
	|| _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED ||  _POSIX_C_SOURCE >= 200809L
		#define _STR_DUP(s)		strdup((s))
	#elif defined  _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700
		#define _STR_DUP(s)		strndup((s),strlen(s)+1)
	#elif _GNU_SOURCE
		#define _STR_DUP(s)		strndupa((s))
	#endif

	#ifdef __USE_GNU 
		#define _ENVIRON	environ
	#else
		#define _ENVIRON	__environ
	#endif 

	#if defined  _BSD_SOURCE &&	\
        !(_POSIX_SOURCE || _POSIX_C_SOURCE || _XOPEN_SOURCE ||	\
        _XOPEN_SOURCE_EXTENDED || _GNU_SOURCE || _SVID_SOURCE)	
        #define _SETPGID(pid,pgid)		setpgrp((pid),(pgid))
	#else	
		#define _SETPGID(pid,pgid)		setpgid((pid),(pgid))
	#endif

	typedef struct job_st {
		char *name;					/* Имя команды */
		int (*handler)(void *);  	/* Обработчик */
	} job;

	typedef struct single_execute {	/* Структура исполняемой единицы */
		char *name;
		char **argv;				/* Аргументы исполняемой единицы */
		char *file;					/* Файлы в которые или из которых идёт в\в данных */
		int8_t mode;				/* Режим перенаправления ввода вывода */
		struct
			single_execute *next;	/* Следующая исполяемая единица в цепочке */		
		int (*handler)(void *prm);	/* Указатель на исполнителя, если он есть */
		pid_t pid;					/* Индетификатор процесса */
		int status;					/* Статус процесса (выполняется или остановлен) */
	} sing_exec;

	job job_sh; 		/* Структура для описания обработчика встроенной функции */

	/* Список выполняемых программ оболочки */
	list_id sh_jobs;

	/* Список программ выполняющихся в фоновом режиме */
	list_id bg_jobs;

	struct queue sp_queue;

	void init_jobs();
	
	void del_jobs();

	/* Добавление специального исмвола в очередь */
	void add_spec(int val);

	/* Взятие специального символа из очереди */
	int  get_spec();

	/* Определение встроенной функции */
	int (* is_shell_cmd(char *cmd)) (void *);

	/* Подготовка аргументов для команды */
	void *prepare_args(int num, sing_exec *ex, unsigned mode, list_id list);

	/* Запуск команды */
	int exec_cmd (sing_exec *ex);

	/* Ожидание дочернего процесса */
	int wait_child(sing_exec *ex);

	/* Обновление списка фоновых процессов */
	void update_jobs();

	/* Содание очереди на исполнение */
	sing_exec *create_exec_queue();

	/* Освобождение памяти, занятую под исп.единицу */
	void free_exec(sing_exec *ex);

	/* Добавление обработчика встроенной функции оболочки */
	#define add_job(n,h) 				\
				job_sh.name = (n); 		\
				job_sh.handler = &(h); 	\
				list_add(&job_sh,sizeof(job),sh_jobs);
				
	#define add_bg_job(proc,stat)								\
					do {										\
						(proc)->status = stat;					\
						list_add(proc,sizeof(sing_exec),bg_jobs);\
					} while (0)

#endif