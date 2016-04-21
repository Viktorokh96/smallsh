#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include "defines.h"
#include "services/bits.h"
#include "services/list.h"
#include "./parses/parse.h"
#include "./jobs/jobs.h"
#include "./signal/signal.h"
#include "general.h"
#include "shell.h"

/* Инициализация служебных систем оболочки */
void init_shell(unsigned mode, char *argv[])
{
	strcpy(shell_name,argv[0]);

	if (bit_seted(mode,SIGNAL)) {
		init_signals();
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
#ifdef LIST_H
	if (bit_seted(mode,LIST)) {
		arg_list = -1;
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

	/* Список всегда должен завершать NULL */
	argv[i] = NULL;

	return argv;
}

int try_exec(char *path) 
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
				sh_handler((void *) prepare_args());	
		} else 
#endif
		{
			pid = fork();
			if (pid == 0) {
				if(try_exec(getenv("PATH")) != 0) 
				if(try_exec(getenv("PWD")) != 0) {
					printf("Исполняемый файл не найден.\n");
					_exit(1);
					
				}

			}
			else {
				if(bit_seted(cmd_type,INCL_BACKGR)) {
				#ifdef JOBS_H
					add_bg_job((char *) list_get(0,arg_list), pid, getppid());
				#endif 
				}
				else wait(&child_stat);
			}
		}
	}
}

/* Получаем команду от пользователя */
inline unsigned get_command(char *cmd)
{
	/* Удаляем старый список */
	if (arg_list != -1)	list_del(arg_list);

	arg_list = init_list();
	
	/* Если имеем дело с устаревшим компилятором */
#if defined USE_DEPRICATED
    gets(cmd);
#else
    fgets(cmd,CMD_SIZE,stdin);
#endif
    return parse_cmd(cmd,arg_list);
}

inline void clear_cmd_buff(char *cmd_buf)
{
	int i;
	for(i = 0; i < CMD_SIZE; i++) cmd_buf[i] = 0; 
}

int main(int argc, char *argv[])
{
	char command[CMD_SIZE];
	unsigned cmd_type;
	
	init_shell(SIGNAL | JOBS | GENERAL | LIST , argv);
	
	for(;;) {
		clear_cmd_buff(command);		 /* Принудительная очистка */
		printf("%s:%s#|>",getlogin(),curr_path);
		cmd_type = get_command(command); /* command более использоваться не должен */
		exec_command(cmd_type);	
	}
	
	return 0;
}

void end_of_work()
{
	list_del(arg_list);
#ifdef JOBS_H
	del_jobs();
#endif
	_exit(1);

}
