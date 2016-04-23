#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
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
	if (bit_seted(mode,JOBS)) {
		init_jobs();
	}
	if (bit_seted(mode,GENERAL)) {
		init_general();
	}
	if (bit_seted(mode,LIST)) {
		arg_list = -1;
	}
}

/* Запуск команды */
void exec_command()
{
	sing_exec *first;

	if((first = create_exec_queue()) != NULL)				/* Создание очереди на исполнение */
		first -> exec_func(first);							/* и непосредственное исполнение */
}

/* Получаем команду от пользователя */
inline char *get_command(char *cmd)
{
	/* Если имеем дело с устаревшим компилятором */
#if defined USE_DEPRICATED
    gets(cmd);
#else
    fgets(cmd,CMD_SIZE,stdin);
#endif
    return cmd;
}

inline void clear_cmd_buff(char *cmd_buf)
{
	int i;
	for(i = 0; i < CMD_SIZE; i++) cmd_buf[i] = 0; 
}

int main(int argc, char *argv[])
{
	char command[CMD_SIZE];
	char *cmd;											/* Указатель на следующую команду */
	
	init_shell(SIGNAL | JOBS | GENERAL | LIST , argv);
	
	for(;;) {
		clear_cmd_buff(command);					 	/* Принудительная очистка */
		printf("%s:%s#|>",getlogin(),short_path(curr_path));
		cmd = get_command(command);			 			/* Выполнение команды */
		while((cmd = parse_cmd(cmd)) != NULL)
			exec_command();
		exec_command();									/* Выполнение последней команды */	
	}
	
	return 0;
}

void end_of_work()
{
	del_general();
	del_jobs();
	_exit(1);
}
