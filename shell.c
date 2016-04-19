#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <signal.h>
#include "defines.h"
#include "services/bits.h"
#include "services/list.h"
#include "./parses/parse.h"
#include "./jobs/jobs.h"
#include "general.h"
#include "shell.h"

/* Определение функции обработчика */
void SigHandler(int sig)
{
	if (getpid() != 0) {
		printf("Shut up honey!\n");
		fflush(stdout);
	}
	return;
}

/* Инициализация служебных систем оболочки */
void init_shell(unsigned mode)
{
	if (bit_seted(mode,SIGNAL)) {
		signal(SIGINT,&SigHandler);
	}
#ifdef JOBS_H
	if (bit_seted(mode,JOBS)) {
		init_jobs();
	}
#endif
#ifdef GENERAL_H
	if (bit_seted(mode,GENERAL)) {
		init_general();
	}
#endif
}

/* Подготавливаем аргументы */
char **prepare_args()
{
	char **argv;
	int i, size;

	argv = malloc(list_count(arg_list)*sizeof(char *));
	for (i = 0; i < list_count(arg_list); i++) {
		size = strlen((char *)list_get(i,arg_list))+1;
		argv[i] = malloc(size*sizeof(char));
		memcpy(argv[i],(char *)list_get(i,arg_list),size);
	}

	argv[i] = NULL;

	return argv;
}

inline int try_exec_with_args(char *path) 
{
	int state = 0;
	char *tmp;
	char *exec_path = strdup(path);
	
	while((tmp = make_exec_path(&exec_path,
		(char *) list_get(0,arg_list))) != NULL) {
		if (exec_path == NULL) state = 1;
	#ifdef __USE_GNU
		if(!execve(exec_path,prepare_args(),environ)) return 0;
		else state = 1;
	#else
		if(!execve(exec_path,prepare_args(),__environ)) return 0;
		else state = 1;
	#endif
		exec_path = tmp;
	}
	return state;
}

inline int try_exec(char *path) 
{
	int state = 0;
	char *tmp;
	char *exec_path = strdup(path);
	
	while((tmp = make_exec_path(&exec_path,
		(char *) list_get(0,arg_list))) != NULL) {
		if (exec_path == NULL) state = 1;
		/*printf("%s\n",tmp );*/				/* DEBUG */
		if (!execl(exec_path,"",NULL)) return 0;
		else state = 1;
		exec_path = tmp;
	} 
	return state;
}

/* Запуск команды */
void exec_command(unsigned cmd_type)
{
	pid_t pid;
	int child_stat;
	int (*sh_handler)(void *) = NULL;

	if (bit_seted(cmd_type,INCL_EXEC)) {
#ifdef JOBS_H
		/* Если такая команда встроена в оболочку */
		if ((sh_handler = is_shell_cmd((char *) list_get(0,arg_list))) != NULL) {
			if(bit_seted(cmd_type,INCL_ARGS))
				sh_handler((void *) prepare_args());	
			else sh_handler(NULL);
		} else 
#endif
		{
			pid = fork();
			if (pid == 0) {
				if(!bit_seted(cmd_type,INCL_ARGS)) {					
					if(try_exec(getenv("PATH")) != 0) 
					if(try_exec(getenv("PWD")) != 0) {
						printf("Исполняемый файл не найден.\n");
						_exit(1);
					}
				} else {
					if(try_exec_with_args(getenv("PATH")) != 0) 
					if(try_exec_with_args(getenv("PWD")) != 0) {
						printf("Исполняемый файл не найден.\n");
						_exit(1);
					}
				}

			}
			else {
				wait(&child_stat);
			}
		}
	}
}

/* Получаем команду от пользователя */
inline unsigned get_command(char *cmd)
{
	arg_list = init_list();
	/* Если имеем дело с устаревшим компилятором */
#if defined USE_DEPRICATED
    gets(cmd);
#else
    fgets(cmd,CMD_SIZE,stdin);
#endif
    return parse_cmd(cmd,arg_list);
}

int main(int argc, char *argv[])
{
	char command[CMD_SIZE];
	unsigned cmd_type;
	
	init_shell(SIGNAL | JOBS | GENERAL);
	
	for(;;) {
		printf("%s:%s#|>",getlogin(),curr_path);
		cmd_type = get_command(command); /* command более использоваться не должен */
		exec_command(cmd_type);	
		command[0] = '\0';
		list_del(arg_list);
	}
	
	return 0;
}

void end_of_work()
{
	list_del(arg_list);
#ifdef JOBS_H
	del_jobs();
#endif
}
