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

    #if defined _XOPEN_SOURCE >= 500 || _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED \
    ||  _POSIX_C_SOURCE >= 200809L
    	#define GET_PGID(pid)			getpgid((pid))
    #else
    	#define GET_PGID(pid)			getpgrp((pid))
    #endif


    /* Состояния задания */
	#define	TSK_RUNNING	1
	#define TSK_STOPPED	2
	#define TSK_KILLED	3
   	#define TSK_EXITED	4

    /* Режимы выполнения задания */
	#define RUN_ACTIVE		1		/* Задача требует запуска в активном режиме */
	#define RUN_BACKGR		2		/* Задача требует запуска в фоновом режиме */

    #define PASS_BACKGR		-1		/* От exec_cmd означает что процесс выполняется в фоновом режиме */
    #define EMPTY_EX		-2		/* Если пустая команда */

    /* Возможные специальные условия выполнения */
	#define NO_EX			0
	#define AND_EX			1		/* && */
	#define OR_EX			2		/* || */
    #define PIPE_EX			3		/* | */

    #define NO_NEXT 		2
    #define NORMAL_NEXT 	1

    typedef struct st_task task;

	typedef struct job_st {
		char *name;					/* Имя команды */
		int (*handler)(void *);  	/* Обработчик */
	} job;

	typedef struct single_execute {	/* Структура исполняемой единицы */
		char *name;
		char **argv;				/* Аргументы исполняемой единицы */
		int8_t ex_mode;				/* Условие выполнения следующего задания ('&&','||','|' или 0 если нет) */
#if 1		
		char *file;					/* Файлы в которые или из которых идёт в\в данных */
		int8_t ios;					/* Режим перенаправления ввода вывода */
#endif
		struct
			single_execute *next;	/* Следующая исполяемая единица в цепочке */		
		int (*handler)(void *prm);	/* Указатель на исполнителя, если он есть */
		pid_t pid;					/* Индетификатор процесса */
		task *tsk;					/* Задание в рамках которого выполнется данная исп.еденица */
	} sing_exec;

	typedef struct st_task {		/* Структура задания */
		char *name;
		pid_t gpid;					/* Индетификатор задания (группы процессов) */
		int status;					/* Статус задания (выполняется или остановлен) */
		int8_t mode;				/* Режим выполнения (фоновый или активный) */
        int stdin, stdout, stderr; 
		sing_exec *first;
		sing_exec *current_ex;
	} task;

	job job_sh; 		/* Структура для описания обработчика встроенной функции */

	/* Список выполняемых программ оболочки */
	list_id sh_jobs;

	/* Список программ выполняющихся в фоновом режиме */
	list_id bg_jobs;


	void init_jobs();
	
	void del_jobs();

	/* Определение встроенной функции */
	int (* is_shell_cmd(char *cmd)) (void *);

	/* Подготовка аргументов для команды */
	void *prepare_args(int num, sing_exec *ex, unsigned mode, list_id list);

	/* Запуск команды в режимах: NORMAL_NEXT, NO_NEXT */
	int exec_cmd (sing_exec *ex,int8_t mode);

	/* Ожидание дочернего процесса */
	int wait_child(sing_exec *ex);

	/* Обновление списка фоновых процессов */
	void update_jobs();

	/* Содание очереди на исполнение */
	sing_exec *create_exec_queue(task *tsk);

	/* Инициализация команды */
	sing_exec *make_sing_exec(task *tsk,int num,list_id arg_list);

	/* Создание нового задания */
	task *create_task();

	/* Проверка статуса с последующим выполнением (возвращает 1 если next = NULL) */
	void exec_next(sing_exec *ex, int stat);

	/* Запуск выполнения задания */
	int exec_task(task *tsk);

	/* Освобождение памяти, занятую под исп.единицу */
	void free_exec(sing_exec *ex);

	/* Освобождение памяти, занятую под задание */
	void destroy_task(task *tsk);

	/* Добавление обработчика встроенной функции оболочки */
	#define add_job(n,h) 				\
				job_sh.name = (n); 		\
				job_sh.handler = &(h); 	\
				list_add(&job_sh,sizeof(job),sh_jobs);
				
	#define add_bg_task(tsk,stat)								\
				do {											\
					(tsk)->status = stat;						\
					list_add((tsk),sizeof(task),bg_jobs);		\
				} while (0)

#endif