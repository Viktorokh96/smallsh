#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include "../services/table.h"
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

    /* Task status */
#define	TSK_RUNNING		1
#define TSK_STOPPED		2
#define TSK_KILLED		3
#define TSK_EXITED		4

    /* Task run modes */
#define RUN_ACTIVE		1	/* Task will be executed in active (foreground) mode */
#define RUN_BACKGR		2	/* Task will be executed in background mode */

#define PASS_BACKGR		-1
#define EMPTY_EX		-2	/* If empty command */

    /* Possible special conditions of execution */
#define NO_EX			0
#define AND_EX			1	/* && */
#define OR_EX			2	/* || */
#define PIPE_EX			3	/* | */

    /* IO Redirection modes */
#define IO_IN			1
#define IO_OUT			2

#define NO_NEXT 		2
#define NORMAL_NEXT 	1

typedef struct st_task task;

typedef struct job_st {
	char *name;		/* Name of command */
	int (*handler) (void *);	/* Handler */
} job;

typedef struct single_execute {	/* Single execution struct ( process synonym ) */
	char *name;
	char **argv;		/* Arguments of ex. */
	int8_t ex_mode;		/* Condition for next running ('&&','||','|' or 0 if not has) */
#if 1
	char *file;		/* Files that used fo IO ( if has switching ) */
	int8_t ios;		/* IO switch mode */
#endif
	struct
	single_execute *next;	/* Next single execution in chain */
	int (*handler) (void *prm);	/* Pointer to handler if it has */
	pid_t pid;		/* Process ID */
	task *tsk;		/* The task within which this process is performed */
} sing_exec;

typedef struct st_task {	/* Task struct */
	char *name;
	pid_t pgid;		/* Task ID (group of process) */
	int status;		/* Task status (running or stopped) */
	int8_t mode;		/* Execution mode (background or foreground) */
	struct termios tmodes;
	int stdin, stdout, stderr;
	sing_exec *first;
	sing_exec *current_ex;
} task;

	/* Table of built-in commands */
addr_table sh_jobs;

	/* The table of the tasks which are executed in the background */
addr_table bg_jobs;

	/* Table of arguments in command */
addr_table arg_vec;

void init_jobs();

void del_jobs();

	/* Find built-in commands */
int (*is_shell_cmd(char *cmd)) (void *);

	/* Execute commands in modes: NORMAL_NEXT, NO_NEXT */
int exec_cmd(sing_exec * ex, int8_t mode);

	/* Waiting for child process */
int wait_child(sing_exec * ex);

	/* Updating a table of background process */
void update_jobs();

	/* creating a new executable queue */
sing_exec *create_exec_queue(task * tsk);

	/* Create a new task */
task *create_task();

	/* Checking status and execute next */
void exec_next(sing_exec * ex, int stat);

	/* Execute group of processes */
int exec_task(task * tsk);

	/* Freeing allocated memory for single execution */
void free_exec(sing_exec * ex);

	/* Freeing allocated memory for task */
void destroy_task(task * tsk);

	/* To find process in task */
sing_exec *have_ex(task * tsk, pid_t pid);

void set_task_to_term(task * tsk);

void unset_task_from_term(task * tsk);

#define add_bg_task(tsk,stat)								\
				do {											\
					(tsk)->mode = RUN_BACKGR;					\
					(tsk)->status = stat;						\
					table_add((tsk),&bg_jobs);		\
				} while (0)

#endif
